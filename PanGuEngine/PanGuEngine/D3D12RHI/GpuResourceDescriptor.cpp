#include "pch.h"
#include "GpuResourceDescriptor.h"
#include "RenderDevice.h"

namespace RHI
{

	GpuResourceDescriptor::GpuResourceDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type, std::shared_ptr<GpuResource> OwnResource) :
		m_Resource(OwnResource)
	{
		// TODO: ����Ƿ���õ��ƶ���ֵ�����
		m_Allocation = RenderDevice::GetSingleton().AllocateDescriptor(Type);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GpuResourceDescriptor::GetCpuHandle() const
	{
		return m_Allocation.GetCpuHandle(0);
	}

}
