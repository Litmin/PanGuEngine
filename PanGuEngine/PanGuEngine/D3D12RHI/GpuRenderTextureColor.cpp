#include "pch.h"
#include "GpuRenderTextureColor.h"
#include "RenderDevice.h"

namespace RHI
{

	RHI::GpuRenderTextureColor::GpuRenderTextureColor(UINT32 width, UINT32 height, 
													  D3D12_RESOURCE_DIMENSION dimension, 
													  DXGI_FORMAT format, 
													  Color clearColor) :
		GpuRenderTexture(width, height, dimension, format),
		m_ClearColor(clearColor)
	{

		D3D12_RESOURCE_DESC Desc = {};
		Desc.Alignment = 0;
		Desc.DepthOrArraySize = 1;
		Desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		Desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		Desc.Format = GetBaseFormat(format);
		Desc.Height = (UINT)m_Height;
		Desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		Desc.MipLevels = 0;
		Desc.SampleDesc.Count = 1;
		Desc.SampleDesc.Quality = 0;
		Desc.Width = (UINT64)m_Width;

		D3D12_CLEAR_VALUE ClearValue = {};
		ClearValue.Format = format;
		ClearValue.Color[0] = m_ClearColor.R();
		ClearValue.Color[1] = m_ClearColor.G();
		ClearValue.Color[2] = m_ClearColor.B();
		ClearValue.Color[3] = m_ClearColor.A();

		CD3DX12_HEAP_PROPERTIES HeapProps(D3D12_HEAP_TYPE_DEFAULT);

		m_UsageState = D3D12_RESOURCE_STATE_COMMON;

		ThrowIfFailed(RenderDevice::GetSingleton().GetD3D12Device()->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE,
			&Desc, m_UsageState, &ClearValue, IID_PPV_ARGS(&m_pResource)));
	}

	std::shared_ptr<GpuResourceDescriptor> GpuRenderTextureColor::CreateSRV()
	{

	}

	std::shared_ptr<GpuResourceDescriptor> GpuRenderTextureColor::CreateRTV()
	{

	}

	std::shared_ptr<GpuResourceDescriptor> GpuRenderTextureColor::CreateUAV()
	{

	}

}

