#pragma once
#include "GpuResource.h"
#include "DescriptorHeap.h"

namespace RHI 
{
    /**
    * ��ʾ��Դ��Descriptor����RTV��DSV����������CPUDescriptorHeap�з����
    */
    class GpuResourceDescriptor
    {
    public:
        GpuResourceDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type);

        D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle() const;
        // ��Դ��Descriptorֻ������CPU Descriptor Heap�У��󶨵����ߵ�GPU Descriptor Heap��ShaderResourceCache�з���
        //D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() const;

        const GpuResource* GetResource() const { return m_Resource.get(); }

		bool   IsNull()                 const { return m_Allocation.IsNull(); }
		bool   IsShaderVisible()        const { return m_Allocation.IsShaderVisible(); }

    protected:
        // ����Դӵ������Ȩ��GpuResourceView����ʱ��֤��Դ�����ͷ�
        std::shared_ptr<GpuResource> m_Resource;

        DescriptorHeapAllocation m_Allocation;
    };

}