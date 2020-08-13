#include "pch.h"
#include "Engine.h"

using namespace std;
using Microsoft::WRL::ComPtr;
using namespace DirectX;

Engine* Engine::m_Engine = nullptr;
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    return Engine::Get()->MsgProc(hWnd, message, wParam, lParam);
}

Engine::Engine()
{
    assert(m_Engine == nullptr);
    m_Engine = this;
}

Engine::~Engine()
{
    if (m_Device != nullptr)
        FlushCommandQueue();
}

void Engine::Initialize(UINT width, UINT height, HINSTANCE hInstance)
{
    m_Width = width;
    m_Height = height;
    m_AppInst = hInstance;
    m_Aspect = static_cast<float>(width) / static_cast<float>(height);
    m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    m_ScissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(width), static_cast<LONG>(height));
    InitialMainWindow();
	InitialDirect3D();

    OnResize();
    
    // Initilize Managers
    m_ResourceManager = make_unique<ResourceManager>();
    m_SceneManager = make_unique<SceneManager>();
    m_ShaderManager = make_unique<ShaderManager>();

    m_GraphicContext = make_unique<GraphicContext>(m_Device.Get(), m_CommandList.Get(), m_CommandAllocator.Get(), m_CommandQueue.Get(), m_Fence.Get(), m_CbvSrvUavDescriptorSize);

    m_Initialized = true;
}

int Engine::Run()
{
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        // Process any messages in the queue.
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        Tick();
    }

    Destroy();

    return (int)msg.wParam;
}

void Engine::Tick()
{
	Update();
	Render();
}

void Engine::Update()
{
    // CPU、GPU同步
    m_GraphicContext->Update();
    // 更新Constant Buffer
    m_SceneManager->UpdateTransform();
    m_SceneManager->UpdateRendererCBs();
    m_SceneManager->UpdateMainPassBuffer();
}

void Engine::Render()
{
    auto cmdListAlloc = m_GraphicContext->GetCurrFrameResource()->m_CmdListAlloc;

    // Reuse the memory associated with command recording.
    // We can only reset when the associated command lists have finished execution on the GPU.
    ThrowIfFailed(cmdListAlloc->Reset());

    ThrowIfFailed(m_CommandList->Reset(cmdListAlloc.Get(), nullptr));

    m_CommandList->RSSetViewports(1, &m_Viewport);
    m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

    // Indicate a state transition on the resource usage.
    m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    // Clear the back buffer and depth buffer.
    m_CommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
    m_CommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    // Specify the buffers we are going to render to.
    m_CommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

    // 渲染场景
    m_SceneManager->Render();

    // Indicate a state transition on the resource usage.
    m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    // Done recording commands.
    ThrowIfFailed(m_CommandList->Close());

    ID3D12CommandList* ppCommandLists[] = { m_CommandList.Get() };
    m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present the frame.
    ThrowIfFailed(m_SwapChain->Present(1, 0));
    m_CurrBackBuffer = (m_CurrBackBuffer + 1) % SwapChainBufferCount;

    // Advance the fence value to mark commands up to this fence point.
    m_GraphicContext->GetCurrFrameResource()->Fence = ++m_FenceValue;

    // Add an instruction to the command queue to set a new fence point. 
    // Because we are on the GPU timeline, the new fence point won't be 
    // set until the GPU finishes processing all the commands prior to this Signal().
    m_CommandQueue->Signal(m_Fence.Get(), m_FenceValue);
}

LRESULT Engine::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (m_Initialized)
    {
        switch (msg)
        {
            // WM_ACTIVATE is sent when the window is activated or deactivated.  
            // We pause the game when the window is deactivated and unpause it 
            // when it becomes active.  
        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE)
            {
                // Pause
                Pause();
            }
            else
            {
                // Resume
                Resume();
            }
            return 0;

            // WM_SIZE is sent when the user resizes the window.  
        case WM_SIZE:
            // Save the new client area dimensions.
            SetScreenSize(LOWORD(lParam), HIWORD(lParam));
            if (wParam == SIZE_MINIMIZED)
            {
                Pause();
                m_minimized = true;
                m_maximized = false;
            }
            else if (wParam == SIZE_MAXIMIZED)
            {
                Resume();
                m_minimized = false;
                m_maximized = true;
                OnResize();
            }
            else if (wParam == SIZE_RESTORED)
            {

                // Restoring from minimized state?
                if (m_minimized)
                {
                    Resume();
                    m_minimized = false;
                    OnResize();
                }

                // Restoring from maximized state?
                else if (m_maximized)
                {
                    Resume();
                    m_maximized = false;
                    OnResize();
                }
                else if (m_resizing)
                {
                    // If user is dragging the resize bars, we do not resize 
                    // the buffers here because as the user continuously 
                    // drags the resize bars, a stream of WM_SIZE messages are
                    // sent to the window, and it would be pointless (and slow)
                    // to resize for each WM_SIZE message received from dragging
                    // the resize bars.  So instead, we reset after the user is 
                    // done resizing the window and releases the resize bars, which 
                    // sends a WM_EXITSIZEMOVE message.
                }
                else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
                {
                    OnResize();
                }
            }
            return 0;

            // WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
        case WM_ENTERSIZEMOVE:
            Pause();
            m_resizing = true;
            return 0;

            // WM_EXITSIZEMOVE is sent when the user releases the resize bars.
            // Here we reset everything based on the new window dimensions.
        case WM_EXITSIZEMOVE:
            Resume();
            m_resizing = false;
            OnResize();
            return 0;

            // WM_DESTROY is sent when the window is being destroyed.
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

            // The WM_MENUCHAR message is sent when a menu is active and the user presses 
            // a key that does not correspond to any mnemonic or accelerator key. 
        case WM_MENUCHAR:
            // Don't beep when we alt-enter.
            return MAKELRESULT(0, MNC_CLOSE);

            // Catch this message so to prevent the window from becoming too small.
        case WM_GETMINMAXINFO:
            ((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
            ((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
            return 0;

            /*case WM_LBUTTONDOWN:
            case WM_MBUTTONDOWN:
            case WM_RBUTTONDOWN:
                OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
                return 0;
            case WM_LBUTTONUP:
            case WM_MBUTTONUP:
            case WM_RBUTTONUP:
                OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
                return 0;
            case WM_MOUSEMOVE:
                OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
                return 0;
            case WM_KEYUP:
                if (wParam == VK_ESCAPE)
                {
                    PostQuitMessage(0);
                }
                else if ((int)wParam == VK_F2)
                    Set4xMsaaState(!m4xMsaaState);

                return 0;*/
        }
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void Engine::Destroy()
{

}

void Engine::Pause()
{
}

void Engine::Resume()
{
}


void Engine::InitialDirect3D()
{
    // 开启调试
#if defined(DEBUG) || defined(_DEBUG) 
    {
        ComPtr<ID3D12Debug> debugController;
        ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
        debugController->EnableDebugLayer();
    }
#endif

    ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_DXGIFactory)));

    ThrowIfFailed(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_Device)));

    ThrowIfFailed(m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));

    m_RtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_DsvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    m_CbvSrvUavDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    CreateCommandObjects();
    CreateSwapChain();
    CreateRtvAndDsvDescriptorHeaps();
}

void Engine::InitialMainWindow()
{
    //// 命令行参数
    //int argc;
    //LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    //LocalFree(argv);

    //// 创建Windows窗口
    //WNDCLASSEX windowsClass = { 0 };
    //windowsClass.cbSize = sizeof(WNDCLASSEX);
    //windowsClass.style = CS_HREDRAW | CS_VREDRAW;
    //windowsClass.lpfnWndProc = WindowProc;
    //windowsClass.hInstance = m_AppInst;
    //windowsClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    //windowsClass.lpszClassName = L"PanGu";

    //RegisterClassEx(&windowsClass);

    //RECT windowRect = { 0, 0, static_cast<LONG>(m_Width), static_cast<LONG>(m_Height) };
    //AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    //m_MainWnd = CreateWindow(
    //    windowsClass.lpszClassName,
    //    L"PanGu",
    //    WS_OVERLAPPEDWINDOW,
    //    CW_USEDEFAULT,
    //    CW_USEDEFAULT,
    //    windowRect.right - windowRect.left,
    //    windowRect.bottom - windowRect.top,
    //    nullptr,        // We have no parent window.
    //    nullptr,        // We aren't using menus.
    //    m_AppInst,
    //    nullptr);

    //ShowWindow(m_MainWnd, SW_SHOW);



    WNDCLASS wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = m_AppInst;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = L"MainWnd";

    if (!RegisterClass(&wc))
    {
        MessageBox(0, L"RegisterClass Failed.", 0, 0);
    }

    // Compute window rectangle dimensions based on requested client area dimensions.
    RECT R = { 0, 0, m_Width, m_Height };
    AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
    int width = R.right - R.left;
    int height = R.bottom - R.top;

    m_MainWnd = CreateWindow(L"MainWnd", L"PanGu",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, m_AppInst, 0);
    if (!m_MainWnd)
    {
        MessageBox(0, L"CreateWindow Failed.", 0, 0);
    }

    ShowWindow(m_MainWnd, SW_SHOW);
    UpdateWindow(m_MainWnd);
}

void Engine::CreateCommandObjects()
{
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    ThrowIfFailed(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_CommandQueue.GetAddressOf())));

    ThrowIfFailed(m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_CommandAllocator.GetAddressOf())));

    ThrowIfFailed(m_Device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_CommandAllocator.Get(),
        nullptr,                   
        IID_PPV_ARGS(m_CommandList.GetAddressOf())));

    // 刚创建的CommandList处于记录状态，之后会调用Reset，调用Reset要求CommandList处于关闭状态
    m_CommandList->Close();
}

void Engine::CreateSwapChain()
{
    m_SwapChain.Reset();

    DXGI_SWAP_CHAIN_DESC sd;
    sd.BufferDesc.Width = m_Width;
    sd.BufferDesc.Height = m_Height;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = SwapChainBufferCount;
    sd.OutputWindow = m_MainWnd;
    sd.Windowed = true;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    ThrowIfFailed(m_DXGIFactory->CreateSwapChain(
        m_CommandQueue.Get(),
        &sd,
        m_SwapChain.GetAddressOf()));
}

void Engine::CreateRtvAndDsvDescriptorHeaps()
{
    // Swap Chain使用双缓冲，需要放两个RTV Descriptor
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
    rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;
    ThrowIfFailed(m_Device->CreateDescriptorHeap(
        &rtvHeapDesc, IID_PPV_ARGS(m_RtvHeap.GetAddressOf())));


    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;
    ThrowIfFailed(m_Device->CreateDescriptorHeap(
        &dsvHeapDesc, IID_PPV_ARGS(m_DsvHeap.GetAddressOf())));
}

void Engine::SetScreenSize(UINT width, UINT height)
{
    m_Width = width;
    m_Height = height;
    m_Aspect = static_cast<float>(width) / static_cast<float>(height);
    m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    m_ScissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(width), static_cast<LONG>(height));
}

void Engine::OnResize()
{
    FlushCommandQueue();

    // 修改资源前先同步CPU/GPU
    ThrowIfFailed(m_CommandList->Reset(m_CommandAllocator.Get(), nullptr));

    // 释放资源
    for (int i = 0; i < SwapChainBufferCount; ++i)
        m_SwapChainBuffer[i].Reset();
    m_DepthStencilBuffer.Reset();

    ThrowIfFailed(m_SwapChain->ResizeBuffers(
        SwapChainBufferCount,
        m_Width, m_Height,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

    m_CurrBackBuffer = 0;

    // Render Target View
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT i = 0; i < SwapChainBufferCount; i++)
    {
        ThrowIfFailed(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_SwapChainBuffer[i])));
        m_Device->CreateRenderTargetView(m_SwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
        rtvHeapHandle.Offset(1, m_RtvDescriptorSize);
    }

    // 创建Depth Stencil Buffer
    D3D12_RESOURCE_DESC depthStencilDesc;
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Alignment = 0;
    depthStencilDesc.Width = m_Width;
    depthStencilDesc.Height = m_Height;
    depthStencilDesc.DepthOrArraySize = 1;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE optClear;
    optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    optClear.DepthStencil.Depth = 1.0f;
    optClear.DepthStencil.Stencil = 0;
    ThrowIfFailed(m_Device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &depthStencilDesc,
        D3D12_RESOURCE_STATE_COMMON,
        &optClear,
        IID_PPV_ARGS(m_DepthStencilBuffer.GetAddressOf())));

    // 创建Depth Stencil View
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.Texture2D.MipSlice = 0;
    m_Device->CreateDepthStencilView(m_DepthStencilBuffer.Get(), &dsvDesc, m_DsvHeap->GetCPUDescriptorHandleForHeapStart());

    // 把Depth Stencil Buffer的状态过度为Depth Write
    m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_DepthStencilBuffer.Get(),
        D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));


    ThrowIfFailed(m_CommandList->Close());
    ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
    m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    FlushCommandQueue();

    m_Viewport.TopLeftX = 0;
    m_Viewport.TopLeftY = 0;
    m_Viewport.Width = static_cast<float>(m_Width);
    m_Viewport.Height = static_cast<float>(m_Height);
    m_Viewport.MinDepth = 0.0f;
    m_Viewport.MaxDepth = 1.0f;

    m_ScissorRect = CD3DX12_RECT(0, 0, m_Width, m_Height);
}

ID3D12Resource* Engine::CurrentBackBuffer()const
{
    return m_SwapChainBuffer[m_CurrBackBuffer].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE Engine::CurrentBackBufferView() const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(
        m_RtvHeap->GetCPUDescriptorHandleForHeapStart(),
        m_CurrBackBuffer,
        m_RtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE Engine::DepthStencilView()const
{
    return m_DsvHeap->GetCPUDescriptorHandleForHeapStart();
}

void Engine::FlushCommandQueue()
{
    m_FenceValue++;


    ThrowIfFailed(m_CommandQueue->Signal(m_Fence.Get(), m_FenceValue));

    if (m_Fence->GetCompletedValue() < m_FenceValue)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

        ThrowIfFailed(m_Fence->SetEventOnCompletion(m_FenceValue, eventHandle));

        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }
}


