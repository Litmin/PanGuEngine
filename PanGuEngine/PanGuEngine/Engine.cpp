#include "pch.h"
#include "Engine.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

const int gNumFrameResources = 3;


Engine::~Engine()
{
    FlushCommandQueue();
}

void Engine::Initialize(UINT width, UINT height, HWND hwnd)
{
    m_Width = width;
    m_Height = height;
    m_MainWnd = hwnd;
    m_Aspect = static_cast<float>(width) / static_cast<float>(height);
    m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    m_ScissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(width), static_cast<LONG>(height));
	InitialDirect3D(hwnd);

    OnResize();
    
    m_Initialized = true;
}

void Engine::Tick()
{
	Update();
	Render();
}

// Update Logic
void Engine::Update()
{
    UpdateConstantBuffer();
}



// Render
void Engine::Render()
{
    ThrowIfFailed(m_CommandAllocator->Reset());
    ThrowIfFailed(m_CommandList->Reset(m_CommandAllocator.Get(), nullptr));


    // For each mesh renderer

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { m_CommandList.Get() };
    m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present the frame.
    ThrowIfFailed(m_SwapChain->Present(1, 0));
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


void Engine::InitialDirect3D(HWND hwnd)
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

void Engine::BuildFrameResources()
{
    for (int i = 0; i < gNumFrameResources; ++i)
    {
        m_FrameResources.push_back(std::make_unique<FrameResource>(m_Device.Get(),
            1, (UINT)mAllRitems.size(), (UINT)mMaterials.size()));
    }
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


// TODO: Move To ResourceManager and SceneManager
//********************************************************************************************************
PBRMaterial* Engine::CreateMaterial(float albedo, float metallic, float smoothness)
{
    PBRMaterial* material{ new PBRMaterial{albedo, metallic, smoothness} };
    m_PBRMaterialUpdateConstantQueue.push(material);
    return material;
}

MeshRenderer Engine::CreateRenderer(DirectX::XMFLOAT4X4 worldTransform, Mesh* mesh, PBRMaterial* material)
{
    return MeshRenderer();
}
void Engine::UpdateConstantBuffer()
{
    if (m_Scene != nullptr)
    {

    }
}
//********************************************************************************************************

