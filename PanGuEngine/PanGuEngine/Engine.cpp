#include "pch.h"
#include "Engine.h"
#include "Input.h"

using namespace std;
using namespace DirectX;
using Microsoft::WRL::ComPtr;

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
    // TODO: 释放GPU资源

    RHI::CommandListManager::GetSingleton().IdleGPU();
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

	// 开启调试
#if defined(DEBUG) || defined(_DEBUG) 
	{
		ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif
    ID3D12Device* D3D12Device;
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_DXGIFactory)));
	ThrowIfFailed(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&D3D12Device)));

    m_RenderDevice = make_unique<RHI::RenderDevice>(D3D12Device);
    m_CommandListManager = make_unique<RHI::CommandListManager>(D3D12Device);
    m_CommandContextManager = make_unique<RHI::ContextManager>();

    // SwapChaind要在后面创建，因为构造函数中会分配Descriptor，这需要RenderDevice初始化Descriptor Heap，
    // SwapChadin和Graphic Queue关联
    m_SwapChain = make_unique<RHI::SwapChain>(m_Width, m_Height, m_MainWnd, m_DXGIFactory.Get(), 
        RHI::CommandListManager::GetSingleton().GetGraphicsQueue().GetD3D12CommandQueue());

    
    // Initilize Managers
    m_SceneManager = make_unique<SceneManager>();
    m_vertexFactory = make_unique<VertexFactory>();


    RHI::ShaderCreateInfo shaderCI;
    
    shaderCI.FilePath = L"Shaders\\Standard.hlsl";
    shaderCI.entryPoint = "VS";
    shaderCI.Desc.ShaderType = RHI::SHADER_TYPE_VERTEX;
    m_StandardVS = std::make_shared<RHI::Shader>(shaderCI);

    shaderCI.entryPoint = "PS";
    shaderCI.Desc.ShaderType = RHI::SHADER_TYPE_PIXEL;
    m_StandardPS = std::make_shared<RHI::Shader>(shaderCI);

    RHI::PipelineStateDesc PSODesc;
    PSODesc.PipelineType = RHI::PIPELINE_TYPE_GRAPHIC;
    PSODesc.GraphicsPipeline.VertexShader = m_StandardVS;
    PSODesc.GraphicsPipeline.PixelShader = m_StandardPS;
    PSODesc.GraphicsPipeline.GraphicPipelineState.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    PSODesc.GraphicsPipeline.GraphicPipelineState.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    PSODesc.GraphicsPipeline.GraphicPipelineState.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    PSODesc.GraphicsPipeline.GraphicPipelineState.SampleMask = UINT_MAX;
    PSODesc.GraphicsPipeline.GraphicPipelineState.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    PSODesc.GraphicsPipeline.GraphicPipelineState.NumRenderTargets = 1;
    PSODesc.GraphicsPipeline.GraphicPipelineState.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    PSODesc.GraphicsPipeline.GraphicPipelineState.SampleDesc.Count = 1;
    PSODesc.GraphicsPipeline.GraphicPipelineState.SampleDesc.Quality = 0;
    // TODO: SwapChain的Depth Buffer格式需要跟这里相同
    PSODesc.GraphicsPipeline.GraphicPipelineState.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    // TODO: InputLayout
	//UINT inputLayoutIndex = m_vertexFactory->GetInputLayoutIndex(true, true, true, true, false, false, false);
	UINT inputLayoutIndex = m_vertexFactory->GetInputLayoutIndex(false, true, false, true, false, false, false);
    std::vector<D3D12_INPUT_ELEMENT_DESC>* inputLayoutDesc = m_vertexFactory->GetInputElementDesc(inputLayoutIndex);
    PSODesc.GraphicsPipeline.GraphicPipelineState.InputLayout.NumElements = inputLayoutDesc->size();
    PSODesc.GraphicsPipeline.GraphicPipelineState.InputLayout.pInputElementDescs = inputLayoutDesc->data();

    // shader变量更新频率
	PSODesc.VariableConfig.Variables.push_back(RHI::ShaderResourceVariableDesc{ RHI::SHADER_TYPE_PIXEL, "BaseColorTex", RHI::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE });
	PSODesc.VariableConfig.Variables.push_back(RHI::ShaderResourceVariableDesc{ RHI::SHADER_TYPE_PIXEL, "EmissiveTex", RHI::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE });

    m_PSO = std::make_unique<RHI::PipelineState>(&RHI::RenderDevice::GetSingleton(), PSODesc);

    m_PerDrawCB = std::make_shared<RHI::GpuDynamicBuffer>(1, sizeof(PerDrawConstants));
    m_PerPassCB = std::make_shared<RHI::GpuDynamicBuffer>(1, sizeof(PerPassConstants));
    m_LightCB = std::make_shared<RHI::GpuDynamicBuffer>(1, sizeof(LightConstants));

    RHI::ShaderVariable* perDrawVariable = m_PSO->GetStaticVariableByName(RHI::SHADER_TYPE_VERTEX, "cbPerObject");
    perDrawVariable->Set(m_PerDrawCB);
    RHI::ShaderVariable* perPassVariable = m_PSO->GetStaticVariableByName(RHI::SHADER_TYPE_VERTEX, "cbPass");
    perPassVariable->Set(m_PerPassCB);
    RHI::ShaderVariable* lightVariable = m_PSO->GetStaticVariableByName(RHI::SHADER_TYPE_PIXEL, "cbLight");
    if(lightVariable != nullptr)
        lightVariable->Set(m_LightCB);

    m_Initialized = true;
}

int Engine::Run()
{
    m_Timer.Reset();

    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        m_Timer.Tick();
        Tick();
    }

    Destroy();

    return (int)msg.wParam;
}

void Engine::Tick()
{
    if (!m_Paused)
    {
	    Update(m_Timer.DeltaTime());
	    Render();
    }
}

void Engine::Update(float deltaTime)
{
    CalculateFrameStats();

    m_SceneManager->UpdateCameraMovement(deltaTime);
    
    Input::Update();
}

void Engine::Render()
{
	RHI::GraphicsContext& graphicContext = RHI::GraphicsContext::Begin(L"ForwardRenderer");

    graphicContext.SetViewport(m_Viewport);
    graphicContext.SetScissor(m_ScissorRect);

    RHI::GpuResource* backBuffer = m_SwapChain->GetCurBackBuffer();
    RHI::GpuResourceDescriptor* backBufferRTV = m_SwapChain->GetCurBackBufferRTV();
    RHI::GpuResource* depthStencilBuffer = m_SwapChain->GetDepthStencilBuffer();
    RHI::GpuResourceDescriptor* depthStencilBufferDSV = m_SwapChain->GetDepthStencilDSV();

    graphicContext.TransitionResource(*backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
    graphicContext.TransitionResource(*depthStencilBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    graphicContext.ClearColor(*backBufferRTV, Colors::LightSteelBlue);
    graphicContext.ClearDepthAndStencil(*depthStencilBufferDSV);
    graphicContext.SetRenderTargets(1, &backBufferRTV, depthStencilBufferDSV);

    // TODO: 测试是否只需要绑定一次Descriptor Heap
    ID3D12DescriptorHeap* cbvsrvuavHeap = RHI::RenderDevice::GetSingleton().GetGPUDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).GetD3D12DescriptorHeap();
    ID3D12DescriptorHeap* samplerHeap = RHI::RenderDevice::GetSingleton().GetGPUDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER).GetD3D12DescriptorHeap();
    graphicContext.SetDescriptorHeap(cbvsrvuavHeap, samplerHeap);

    graphicContext.SetPipelineState(m_PSO.get());

    void* pPerPassCB = m_PerPassCB->Map(graphicContext, 256);
    Camera* camera = m_SceneManager->GetCamera();
    camera->UpdateCameraCBs(pPerPassCB);

    LightConstants lightData;
    lightData.LightColor = XMFLOAT3(1.0f, 1.0f, 1.0f);
    lightData.LightDir = XMFLOAT3(-1.0f, 1.0f, 0.0f);
    lightData.LightIntensity = 1.0f;
    void* pLightCB = m_LightCB->Map(graphicContext, 256);
    memcpy(pLightCB, &lightData, sizeof(LightConstants));

    PhongMaterialConstants matConstants;
    matConstants.AmbientStrength = 0.1f;

	// 渲染场景
    const std::vector<MeshRenderer*>& drawList = m_SceneManager->GetDrawList();
    for (INT32 i = 0; i < drawList.size(); ++i)
    {
        // TODO: 优化代码结构
        drawList[i]->GetMaterial()->CreateSRB(m_PSO.get());

        void* pPerDrawCB = m_PerDrawCB->Map(graphicContext, 256);
        drawList[i]->Render(graphicContext, pPerDrawCB);
    }

    graphicContext.TransitionResource(*backBuffer, D3D12_RESOURCE_STATE_PRESENT);

    graphicContext.Finish();

    // TODO: 可以把过度到Present状态封装到Present函数中，函数内部再开始一个GraphicContext
    m_SwapChain->Present();
}

LRESULT Engine::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (m_Initialized)
    {
        switch (msg)
        {
        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE)
            {
                Pause();
            }
            else
            {
                Resume();
            }
            return 0;

        case WM_SIZE:
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

                if (m_minimized)
                {
                    Resume();
                    m_minimized = false;
                    OnResize();
                }

                else if (m_maximized)
                {
                    Resume();
                    m_maximized = false;
                    OnResize();
                }
                else if (m_resizing)
                {
                }
                else 
                {
                    OnResize();
                }
            }
            return 0;

        case WM_ENTERSIZEMOVE:
            Pause();
            m_resizing = true;
            return 0;

        case WM_EXITSIZEMOVE:
            Resume();
            m_resizing = false;
            OnResize();
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_MENUCHAR:
            return MAKELRESULT(0, MNC_CLOSE);

        case WM_GETMINMAXINFO:
            ((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
            ((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
            return 0;

        case WM_LBUTTONDOWN:
            Input::m_KeyStates[(int)KeyCode::Mouse0] = KeyState::KeyDown;
            return 0;

        case WM_MBUTTONDOWN:
            Input::m_KeyStates[(int)KeyCode::Mouse1] = KeyState::KeyDown;
            return 0;

        case WM_RBUTTONDOWN:
            Input::m_KeyStates[(int)KeyCode::Mouse2] = KeyState::KeyDown;
            return 0;

        case WM_LBUTTONUP:
            Input::m_KeyStates[(int)KeyCode::Mouse0] = KeyState::KeyUp;
            return 0;

        case WM_MBUTTONUP:
            Input::m_KeyStates[(int)KeyCode::Mouse1] = KeyState::KeyUp;
            return 0;

        case WM_RBUTTONUP:
            Input::m_KeyStates[(int)KeyCode::Mouse2] = KeyState::KeyUp;
            return 0;

        case WM_MOUSEMOVE:
            Input::OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;

        case WM_KEYDOWN:
            Input::OnKeyDown(wParam);
            return 0;

        case WM_KEYUP:
            if (wParam == VK_ESCAPE)
            {
                PostQuitMessage(0);
            }
            Input::OnKeyUp(wParam);
            return 0;
        }
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void Engine::Destroy()
{
}

void Engine::Pause()
{
    m_Paused = true;
    m_Timer.Stop();
}

void Engine::Resume()
{
    m_Paused = false;
    m_Timer.Start();
}

void Engine::InitialMainWindow()
{
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

    RECT R = { 0, 0, m_Width, m_Height };
    AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
    int width = R.right - R.left;
    int height = R.bottom - R.top;

    wstring m_Title(L"PanGu");
    m_MainWnd = CreateWindow(L"MainWnd", m_Title.c_str(),
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, m_AppInst, 0);
    if (!m_MainWnd)
    {
        MessageBox(0, L"CreateWindow Failed.", 0, 0);
    }

    ShowWindow(m_MainWnd, SW_SHOW);
    UpdateWindow(m_MainWnd);
}

void Engine::SetScreenSize(UINT width, UINT height)
{
    m_Width = width;
    m_Height = height;
    m_Aspect = static_cast<float>(width) / static_cast<float>(height);
    m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    m_ScissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(width), static_cast<LONG>(height));
}

void Engine::CalculateFrameStats()
{
    static int frameCnt = 0;
    static float timeElapsed = 0.0f;

    frameCnt++;

    // Compute averages over one second period.
    if ((m_Timer.TotalTime() - timeElapsed) >= 1.0f)
    {
        float fps = (float)frameCnt; // fps = frameCnt / 1
        float mspf = 1000.0f / fps;

        wstring fpsStr = to_wstring(fps);
        wstring mspfStr = to_wstring(mspf);

        wstring m_Title(L"PanGu");
        wstring windowText = m_Title +
            L"    fps: " + fpsStr +
            L"   mspf: " + mspfStr;

        SetWindowText(m_MainWnd, windowText.c_str());

        // Reset for next average.
        frameCnt = 0;
        timeElapsed += 1.0f;
    }
}

void Engine::OnResize()
{
    m_SwapChain->Resize(m_Width, m_Height);

    m_Viewport.TopLeftX = 0;
    m_Viewport.TopLeftY = 0;
    m_Viewport.Width = static_cast<float>(m_Width);
    m_Viewport.Height = static_cast<float>(m_Height);
    m_Viewport.MinDepth = 0.0f;
    m_Viewport.MaxDepth = 1.0f;

    m_ScissorRect = CD3DX12_RECT(0, 0, m_Width, m_Height);
}
