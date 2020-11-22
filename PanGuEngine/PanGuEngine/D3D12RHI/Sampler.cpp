#include "pch.h"
#include "Sampler.h"
#include "RenderDevice.h"

namespace RHI
{
	Sampler::Sampler(RenderDevice* renderDevice, 
					 const D3D12_SAMPLER_DESC& samplerDesc) :
		m_RenderDevice{renderDevice},
		m_Desc{samplerDesc}
	{
		auto CPUDescriptorAlloc = m_RenderDevice->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
		m_Descriptor = std::move(CPUDescriptorAlloc);
		m_RenderDevice->GetD3D12Device()->CreateSampler(&m_Desc, m_Descriptor.GetCpuHandle());
	}
}