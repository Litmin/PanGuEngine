#pragma once
#include "DescriptorHeap.h"
#include "IShaderResource.h"

namespace RHI
{
	class RenderDevice;
	class Buffer;

	struct BufferFormat
	{
		VALUE_TYPE ValueType = VT_UNDEFINED;

		// 1,2,3,4.对于Formatted Buffer来说，这个值不能是0.
		UINT8 NumComponents = 0;

		// 对于整数来说，该值表示是否归一化到[-1,1]或[0,1]
		bool IsNormalized = false;
	};

	struct BufferViewDesc
	{
		// View的类型：SRV、UAV
		BUFFER_VIEW_TYPE ViewType = BUFFER_VIEW_UNDEFINED;

		// 该成员只针对Formatted Buffer和Raw Buffer.
		BufferFormat Format;

		// 该View表示的区域在整个Buffer中的偏移
		UINT32 ByteOffset = 0;

		// 该View表示的区域的大小
		UINT32 ByteWidth = 0;
	};

	class BufferView : public IShaderResource
	{
	public:
		BufferView(RenderDevice*			  renderDevice,
				   const BufferViewDesc&	  viewDesc,
				   Buffer*					  buffer,
				   DescriptorHeapAllocation&& allocation);

		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle()
		{
			return m_Descriptor.GetCpuHandle();
		}

		Buffer* GetBuffer() const
		{
			return m_Buffer;
		}

	private:
		// CPUDescriptorHeap的Descriptor，BufferView的析构中调用m_DescriptorHandle的析构时会释放Descriptor
		DescriptorHeapAllocation m_Descriptor;

		BufferViewDesc m_Desc;

		Buffer* const m_Buffer;
	};
}
