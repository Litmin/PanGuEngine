#include "pch.h"
#include "GpuCubemap.h"
#include "RenderDevice.h"
#include "CommandContext.h"

namespace RHI
{

	GpuCubemap::GpuCubemap(UINT32 width, UINT32 height, UINT16 mipCount, DXGI_FORMAT format, D3D12_SUBRESOURCE_DATA* initData) :
		GpuTexture(width, height, D3D12_RESOURCE_DIMENSION_TEXTURE2D, format)
	{
		// ���ȿ�����ʼ����
		m_UsageState = D3D12_RESOURCE_STATE_COPY_DEST;
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
		texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;


		D3D12_HEAP_PROPERTIES HeapProps;
		HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		HeapProps.CreationNodeMask = 1;
		HeapProps.VisibleNodeMask = 1;

		ThrowIfFailed(RenderDevice::GetSingleton().GetD3D12Device()->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
			m_UsageState, nullptr, IID_PPV_ARGS(&m_pResource)));

		CommandContext::InitializeTexture(*this, 1, initData);
	}

	std::shared_ptr<GpuResourceDescriptor> GpuCubemap::CreateSRV()
	{
		std::shared_ptr<GpuResourceDescriptor> descriptor = std::make_shared<GpuResourceDescriptor>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, shared_from_this());

		D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.Format = m_Format;
		// TODO: WTF???
		SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;

		SRVDesc.TextureCube.MostDetailedMip = 0;
		SRVDesc.TextureCube.MipLevels = m_MipLevels;
		SRVDesc.TextureCube.ResourceMinLODClamp = 0.0f;

		RenderDevice::GetSingleton().GetD3D12Device()->CreateShaderResourceView(m_pResource.Get(), &SRVDesc, descriptor->GetCpuHandle());

		return descriptor;
	}

}

