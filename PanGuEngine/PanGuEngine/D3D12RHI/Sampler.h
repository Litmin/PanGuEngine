#pragma once
#include "DescriptorHeap.h"

namespace RHI 
{
    class RenderDevice;

    /**
    * DX12中的Sampler，注意不是直接绑定在根签名上的Static Sampler
    */
    class Sampler
    {
    public:
        Sampler(RenderDevice* renderDevice,
                const D3D12_SAMPLER_DESC& samplerDesc);

        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle() { return m_Descriptor.GetCpuHandle(); }


    protected:
        D3D12_SAMPLER_DESC  m_Desc;

        DescriptorHeapAllocation m_Descriptor;

        RenderDevice* m_RenderDevice;
    };

}