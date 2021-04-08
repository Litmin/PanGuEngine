#pragma once
#include "GpuResourceDescriptor.h"
#include "GpuRenderTextureColor.h"
#include "GpuRenderTextureDepth.h"

namespace RHI
{

	class CommandContext;

	class SwapChain
	{
	public:
		SwapChain(UINT32 width, UINT32 height, HWND mainWnd, IDXGIFactory4* dxgiFactory, ID3D12CommandQueue* commandQueue);

		void Resize(UINT32 width, UINT32 height);

		// Present之前需要把Back Buffer过度到Present状态
		void Present();

		const CD3DX12_VIEWPORT& GetViewport() const { return m_Viewport; }
		const CD3DX12_RECT& GetScissorRect() const { return m_ScissorRect; }

		// 在SetRenderTarget时，需要过度到Render Target和Depth Write状态
		GpuRenderTextureColor* GetCurBackBuffer() { return m_BackColorBuffers[m_CurrBackBuffer].get(); }
		GpuRenderTextureDepth* GetDepthStencilBuffer() { return m_DepthStencilBuffer.get(); }
		GpuResourceDescriptor* GetCurBackBufferRTV() { return m_BackColorBufferRTVs[m_CurrBackBuffer].get(); }
		GpuResourceDescriptor* GetDepthStencilDSV() { return m_DepthStencilBufferDSV.get(); }

	private:
		static constexpr UINT32 SwapChainBufferCount = 2;

		Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;

		CD3DX12_VIEWPORT m_Viewport;
		CD3DX12_RECT m_ScissorRect;

		UINT32 m_CurrBackBuffer = 0;
		std::shared_ptr<GpuRenderTextureColor> m_BackColorBuffers[SwapChainBufferCount] = {nullptr};
		std::shared_ptr<GpuRenderTextureDepth> m_DepthStencilBuffer = nullptr;
		std::shared_ptr<GpuResourceDescriptor> m_BackColorBufferRTVs[SwapChainBufferCount] = {nullptr};
		std::shared_ptr<GpuResourceDescriptor> m_DepthStencilBufferDSV = nullptr;
	};

}



