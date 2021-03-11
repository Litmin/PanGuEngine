#include "pch.h"
#include "GpuBuffer.h"
#include "RenderDevice.h"
#include "CommandContext.h"

namespace RHI
{

	GpuBuffer::GpuBuffer(const std::wstring& name, UINT32 NumElements, UINT32 ElementSize, const void* initialData)
	{
		m_ElementCount = NumElements;
		m_ElementSize = ElementSize;
		m_BufferSize = NumElements * ElementSize;

		m_UsageState = D3D12_RESOURCE_STATE_COMMON;


		D3D12_RESOURCE_DESC ResourceDesc = DescribeBuffer();

		D3D12_HEAP_PROPERTIES HeapProps;
		HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		HeapProps.CreationNodeMask = 1;
		HeapProps.VisibleNodeMask = 1;

		ID3D12Device* d3dDevice = RenderDevice::GetSingleton().GetD3D12Device();

		ThrowIfFailed(d3dDevice->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE,
			&ResourceDesc, m_UsageState, nullptr, IID_PPV_ARGS(&m_pResource)));

		m_GpuVirtualAddress = m_pResource->GetGPUVirtualAddress();

		// ����ṩ�˳�ʼ���ݾͻ�������ϴ���Upload���У�Ȼ��Copy��Buffer
		if (initialData)
			CommandContext::InitializeBuffer(*this, initialData, m_BufferSize);

		m_pResource->SetName(name.c_str());
	}

	GpuBuffer::GpuBuffer(const std::wstring& name, UINT32 NumElements, UINT32 ElementSize, const UploadBuffer& srcData, UINT32 srcOffset)
	{
		m_ElementCount = NumElements;
		m_ElementSize = ElementSize;
		m_BufferSize = NumElements * ElementSize;

		m_UsageState = D3D12_RESOURCE_STATE_COMMON;


		D3D12_RESOURCE_DESC ResourceDesc = DescribeBuffer();

		D3D12_HEAP_PROPERTIES HeapProps;
		HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		HeapProps.CreationNodeMask = 1;
		HeapProps.VisibleNodeMask = 1;

		ID3D12Device* d3dDevice = RenderDevice::GetSingleton().GetD3D12Device();

		ThrowIfFailed(d3dDevice->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE,
			&ResourceDesc, m_UsageState, nullptr, IID_PPV_ARGS(&m_pResource)));

		m_GpuVirtualAddress = m_pResource->GetGPUVirtualAddress();

		CommandContext::InitializeBuffer(*this, srcData, srcOffset);

		m_pResource->SetName(name.c_str());
	}


	// Offset:��ʼ��ַƫ�ƣ�Size��Buffer�Ĵ�С��Stride��ÿ��Element�Ĵ�С
	D3D12_VERTEX_BUFFER_VIEW GpuBuffer::CreateVBV(size_t Offset, uint32_t Size, uint32_t Stride) const
	{
		D3D12_VERTEX_BUFFER_VIEW VBView;
		VBView.BufferLocation = m_GpuVirtualAddress + Offset;
		VBView.SizeInBytes = Size;
		VBView.StrideInBytes = Stride;
		return VBView;
	}

	D3D12_VERTEX_BUFFER_VIEW GpuBuffer::CreateVBV(size_t BaseVertexIndex /*= 0*/) const
	{
		size_t Offset = BaseVertexIndex * m_ElementSize;
		return CreateVBV(Offset, (uint32_t)(m_BufferSize - Offset), m_ElementSize);
	}

	// Offset:��ʼ��ַƫ�ƣ� Size��Buffer�Ĵ�С�� b32Bit:��ʽ��32λ����16λ
	D3D12_INDEX_BUFFER_VIEW GpuBuffer::CreateIBV(size_t Offset, uint32_t Size, bool b32Bit) const
	{
		D3D12_INDEX_BUFFER_VIEW IBView;
		IBView.BufferLocation = m_GpuVirtualAddress + Offset;
		IBView.Format = b32Bit ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
		IBView.SizeInBytes = Size;
		return IBView;
	}

	D3D12_INDEX_BUFFER_VIEW GpuBuffer::CreateIBV(size_t StartIndex /*= 0*/) const
	{
		size_t Offset = StartIndex * m_ElementSize;
		return CreateIBV(Offset, (uint32_t)(m_BufferSize - Offset), m_ElementSize == 4);
	}

	D3D12_RESOURCE_DESC GpuBuffer::DescribeBuffer()
	{
		assert(m_BufferSize != 0);

		D3D12_RESOURCE_DESC Desc = {};
		Desc.Alignment = 0;
		Desc.DepthOrArraySize = 1;
		Desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		Desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		// Buffer��Format��Unkown
		Desc.Format = DXGI_FORMAT_UNKNOWN;
		Desc.Height = 1;
		Desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		Desc.MipLevels = 1;
		Desc.SampleDesc.Count = 1;
		Desc.SampleDesc.Quality = 0;
		Desc.Width = (UINT64)m_BufferSize;
		return Desc;
	}

	std::shared_ptr<GpuResourceDescriptor> GpuStructuredBuffer::CreateSRV()
	{
		std::shared_ptr<GpuResourceDescriptor> descriptor = std::make_shared<GpuResourceDescriptor>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
		SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SRVDesc.Buffer.NumElements = m_ElementCount;
		SRVDesc.Buffer.StructureByteStride = m_ElementSize;
		SRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		RenderDevice::GetSingleton().GetD3D12Device()->CreateShaderResourceView(m_pResource.Get(), &SRVDesc, descriptor->GetCpuHandle());

		return descriptor;
	}

	std::shared_ptr<GpuResourceDescriptor> GpuStructuredBuffer::CreateUAV()
	{
		std::shared_ptr<GpuResourceDescriptor> descriptor = std::make_shared<GpuResourceDescriptor>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		// TODO: UAV��Ҫʵ��Counter Buffer
// 		D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
// 		UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
// 		UAVDesc.Format = DXGI_FORMAT_UNKNOWN;
// 		UAVDesc.Buffer.CounterOffsetInBytes = 0;
// 		UAVDesc.Buffer.NumElements = m_ElementCount;
// 		UAVDesc.Buffer.StructureByteStride = m_ElementSize;
// 		UAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
// 
// 		RenderDevice::GetSingleton().GetD3D12Device()->CreateUnorderedAccessView(m_pResource.Get(), &UAVDesc, descriptor->GetCpuHandle());

		return descriptor;
	}

}

