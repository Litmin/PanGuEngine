#include "pch.h"
#include "SwapChain.h"
#include "CommandListManager.h"
#include "CommandContext.h"

using namespace Microsoft::WRL;

namespace RHI
{

	SwapChain::SwapChain(UINT32 width, UINT32 height, HWND mainWnd, IDXGIFactory4* dxgiFactory, ID3D12CommandQueue* commandQueue)
	{
		m_SwapChain.Reset();

		DXGI_SWAP_CHAIN_DESC swapchainDesc;
		swapchainDesc.BufferDesc.Width = width;
		swapchainDesc.BufferDesc.Height = height;
		swapchainDesc.BufferDesc.RefreshRate.Numerator = 60;
		swapchainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapchainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapchainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapchainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapchainDesc.SampleDesc.Count = 1;
		swapchainDesc.SampleDesc.Quality = 0;
		swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapchainDesc.BufferCount = SwapChainBufferCount;
		swapchainDesc.OutputWindow = mainWnd;
		swapchainDesc.Windowed = true;
		swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		ThrowIfFailed(dxgiFactory->CreateSwapChain(
			commandQueue,
			&swapchainDesc,
			m_SwapChain.GetAddressOf()));

		Resize(width, height);
	}

	void SwapChain::Resize(UINT32 width, UINT32 height)
	{
		// Resize前需要Idle GPU
		CommandListManager::GetSingleton().IdleGPU();


		// 释放资源
		for (int i = 0; i < SwapChainBufferCount; ++i)
		{
			m_BackColorBuffers[i].reset();
			m_BackColorBufferRTVs[i].reset();
		}
		m_DepthStencilBuffer.reset();
		m_DepthStencilBufferDSV.reset();

		ThrowIfFailed(m_SwapChain->ResizeBuffers(
			SwapChainBufferCount,
			width, height,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

		m_CurrBackBuffer = 0;

		// 从SwapChain的BackBuffer创建GpuRenderTextureColor对象
		for (int i = 0; i < SwapChainBufferCount; ++i)
		{
			ComPtr<ID3D12Resource> backBuffer;
			ThrowIfFailed(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));
			D3D12_RESOURCE_DESC desc = backBuffer->GetDesc();
			m_BackColorBuffers[i] = std::make_shared<GpuRenderTextureColor>(backBuffer.Detach(), desc);
			// 创建RTV
			m_BackColorBufferRTVs[i] = m_BackColorBuffers[i]->CreateRTV();
		}

		// 创建Depth Stencil Buffer
		m_DepthStencilBuffer = std::make_shared<GpuRenderTextureDepth>(width, height, DXGI_FORMAT_R24G8_TYPELESS);
		// 创建DSV
		m_DepthStencilBufferDSV = m_DepthStencilBuffer->CreateDSV();

		CommandListManager::GetSingleton().IdleGPU();

		m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
		m_ScissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(width), static_cast<LONG>(height));
	}

	// Present之前需要把Back Buffer过度到Present状态
	void SwapChain::Present()
	{
		RHI::GpuResource* backBuffer = GetCurBackBuffer();
		RHI::GraphicsContext& presentContext = RHI::GraphicsContext::Begin(L"Present");
		presentContext.TransitionResource(*backBuffer, D3D12_RESOURCE_STATE_PRESENT);
		presentContext.Finish();

		ThrowIfFailed(m_SwapChain->Present(1, 0));

		m_CurrBackBuffer = (m_CurrBackBuffer + 1) % SwapChainBufferCount;
	}

}

