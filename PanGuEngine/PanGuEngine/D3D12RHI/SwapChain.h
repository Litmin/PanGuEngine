#pragma once
#include "GpuResourceDescriptor.h"

namespace RHI
{

	class SwapChain
	{
	public:
		SwapChain(UINT32 width, UINT32 height, HWND mainWnd, IDXGIFactory4* dxgiFactory, ID3D12CommandQueue* commandQueue);

		void Resize(UINT32 width, UINT32 height);

		void Present();

		// 在SetRenderTarget时，需要过度到Render Target状态
		GpuResourceDescriptor* GetCurBackBufferRTV();

		GpuResourceDescriptor* GetDepthStencilDSV();

	private:
		void InitBufferAndDescriptor();

		static constexpr UINT32 SwapChainBufferCount = 2;

		Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;

		UINT32 m_CurrBackBuffer = 0;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_SwapChainBuffer[SwapChainBufferCount];
		Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthStencilBuffer;

	};

}



