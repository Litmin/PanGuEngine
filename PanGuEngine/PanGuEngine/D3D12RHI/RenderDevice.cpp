#include "pch.h"
#include "RenderDevice.h"

using namespace Microsoft::WRL;

namespace RHI
{
	RenderDevice::RenderDevice(ComPtr<ID3D12Device> d3d12Device) :
		m_D3D12Device{d3d12Device},
		m_CPUDescriptorHeaps
		{
			{*this, 100, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE},
			{*this, 100, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_FLAG_NONE},
			{*this, 100, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE},
			{*this, 100, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE}
		},
		m_GPUDescriptorHeaps
		{ 
			{*this, 100, 100, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE},
			{*this, 100, 100, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE}
		}
	{
	}

	RenderDevice::~RenderDevice()
	{
	}

	DescriptorHeapAllocation RenderDevice::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT Count)
	{
		assert(Type >= D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV && Type < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES && "Invalid heap type");
		return m_CPUDescriptorHeaps[Type].Allocate(Count);
	}

	DescriptorHeapAllocation RenderDevice::AllocateGPUDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT Count)
	{
		assert(Type >= D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV && Type <= D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER && "Invalid heap type");
		return m_GPUDescriptorHeaps[Type].Allocate(Count);
	}

	void RenderDevice::PurgeReleaseQueue(bool forceRelease)
	{
		UINT64 completedFenceValue = forceRelease ? std::numeric_limits<UINT64>::max() : m_CommandQueue.GetCompletedFenceValue();

		while (!m_ReleaseQueue.empty())
		{
			auto& FirstObj = m_ReleaseQueue.front();
			if (FirstObj.first <= completedFenceValue)
				m_ReleaseQueue.pop_front();
			else
				break;
		}
	}
}

