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

		// 如果提供了初始数据就会把数据上传到Upload堆中，然后Copy到Buffer
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


	// Offset:起始地址偏移，Size：Buffer的大小，Stride：每个Element的大小
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

	// Offset:起始地址偏移， Size：Buffer的大小， b32Bit:格式是32位还是16位
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
		Desc.Format = DXGI_FORMAT_UNKNOWN;
		Desc.Height = 1;
		Desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		Desc.MipLevels = 1;
		Desc.SampleDesc.Count = 1;
		Desc.SampleDesc.Quality = 0;
		Desc.Width = (UINT64)m_BufferSize;
		return Desc;
	}

	GpuResourceDescriptor GpuStructuredBuffer::CreateSRV()
	{

	}

	GpuResourceDescriptor GpuStructuredBuffer::CreateUAV()
	{

	}

}

