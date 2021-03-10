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
        // ����Դӵ������Ȩ��GpuResourceView����ʱ��֤��Դ�����ͷ�
        std::shared_ptr<GpuResource> m_Resource;

        std::unique_ptr<DescriptorHeapAllocation> m_Allocation;
    };

}