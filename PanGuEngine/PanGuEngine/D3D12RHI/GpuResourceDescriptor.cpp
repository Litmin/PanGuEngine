#include "pch.h"
#include "GpuResourceDescriptor.h"
#include "RenderDevice.h"

namespace RHI
{

	GpuResourceDescriptor::GpuResourceDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type)
	{
		// TODO: 检查是否调用的移动赋值运算符
		m_Allocation = RenderDevice::GetSingleton().AllocateDescriptor(Type);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GpuResourceDescriptor::GetCpuHandle() const
	{
		return m_Allocation.GetCpuHandle(0);
	}

	D3D12_GPU_DESCRIPTOR_HANDLE GpuResourceDescriptor::GetGpuHandle() const
	{
		return m_Allocation.GetGpuHandle(0);
	}

}
