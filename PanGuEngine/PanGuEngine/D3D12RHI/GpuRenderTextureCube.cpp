#include "pch.h"
#include "GpuRenderTextureCube.h"
#include "RenderDevice.h"
#include "GpuResourceDescriptor.h"

namespace RHI
{
	GpuRenderTextureCube::GpuRenderTextureCube(UINT32 width, UINT32 height, UINT16 mipCount, DXGI_FORMAT format, Color clearColor) :
		GpuRenderTexture(width, height, D3D12_RESOURCE_DIMENSION_TEXTURE2D, format)
	{
		// 首先拷贝初始数据
		m_UsageState = D3D12_RESOURCE_STATE_COMMON;
		m_MipLevels = mipCount;

		D3D12_RESOURCE_DESC texDesc = {};
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Alignment = 0;
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.DepthOrArraySize = 6;
		texDesc.MipLevels = mipCount;
		texDesc.Format = format;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;


		D3D12_HEAP_PROPERTIES HeapProps;
		HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		HeapProps.CreationNodeMask = 1;
		HeapProps.VisibleNodeMask = 1;

		D3D12_CLEAR_VALUE ClearValue = {};
		ClearValue.Format = format;
		ClearValue.Color[0] = m_ClearColor.R();
		ClearValue.Color[1] = m_ClearColor.G();
		ClearValue.Color[2] = m_ClearColor.B();
		ClearValue.Color[3] = m_ClearColor.A();

		ThrowIfFailed(RenderDevice::GetSingleton().GetD3D12Device()->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
			m_UsageState, &ClearValue, IID_PPV_ARGS(&m_pResource)));
	}

	std::shared_ptr<GpuResourceDescriptor> GpuRenderTextureCube::CreateSRV()
	{
		std::shared_ptr<GpuResourceDescriptor> descriptor = std::make_shared<GpuResourceDescriptor>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, shared_from_this());

		D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.Format = m_Format;
		// TODO: WTF???
		SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;

		SRVDesc.TextureCube.MostDetailedMip = 0;
		SRVDesc.TextureCube.MipLevels = -1;	// -1表示所有的mipmap
		SRVDesc.TextureCube.ResourceMinLODClamp = 0.0f;

		RenderDevice::GetSingleton().GetD3D12Device()->CreateShaderResourceView(m_pResource.Get(), &SRVDesc, descriptor->GetCpuHandle());

		return descriptor;
	}

	std::shared_ptr<GpuResourceDescriptor> GpuRenderTextureCube::CreateRTV(UINT8 faceIndex, UINT mipLevel)
	{
		std::shared_ptr<GpuResourceDescriptor> descriptor = std::make_shared<GpuResourceDescriptor>(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, shared_from_this());

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Format = m_Format;
		rtvDesc.Texture2DArray.MipSlice = mipLevel;
		rtvDesc.Texture2DArray.PlaneSlice = 0;
		rtvDesc.Texture2DArray.FirstArraySlice = faceIndex;
		rtvDesc.Texture2DArray.ArraySize = 1;

		RenderDevice::GetSingleton().GetD3D12Device()->CreateRenderTargetView(m_pResource.Get(), &rtvDesc, descriptor->GetCpuHandle());


		return descriptor;
	}
}

