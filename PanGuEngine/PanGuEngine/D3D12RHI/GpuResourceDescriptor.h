#pragma once
#include "GpuResource.h"
#include "DescriptorHeap.h"

namespace RHI 
{
    /**
    * 表示资源的Descriptor或者RTV、DSV，所以是在CPUDescriptorHeap中分配的
    */
    class GpuResourceDescriptor
    {
    public:
        GpuResourceDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type);

        D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle() const;
        // 资源的Descriptor只分配在CPU Descriptor Heap中，绑定到管线的GPU Descriptor Heap在ShaderResourceCache中分配
        //D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() const;

        const GpuResource* GetResource() const { return m_Resource.get(); }

		bool   IsNull()                 const { return m_Allocation.IsNull(); }
		bool   IsShaderVisible()        const { return m_Allocation.IsShaderVisible(); }

    protected:
        // 对资源拥有所有权，GpuResourceView存在时保证资源不被释放
        std::shared_ptr<GpuResource> m_Resource;

        DescriptorHeapAllocation m_Allocation;
    };

}