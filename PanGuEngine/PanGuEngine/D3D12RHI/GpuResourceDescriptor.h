#pragma once
#include "GpuResource.h"
#include "DescriptorHeap.h"

namespace RHI 
{
    /**
    * 
    */
    class GpuResourceDescriptor
    {
    public:

        GpuResourceDescriptor();

    protected:
        // 对资源拥有所有权，GpuResourceView存在时保证资源不被释放
        std::shared_ptr<GpuResource> m_Resource;

        std::unique_ptr<DescriptorHeapAllocation> m_Allocation;
    };

}