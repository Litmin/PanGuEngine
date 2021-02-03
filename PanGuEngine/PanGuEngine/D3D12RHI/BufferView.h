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

		// 1,2,3,4.����Formatted Buffer��˵�����ֵ������0.
		UINT8 NumComponents = 0;

		// ����������˵����ֵ��ʾ�Ƿ��һ����[-1,1]��[0,1]
		bool IsNormalized = false;
	};

	struct BufferViewDesc
	{
		// View�����ͣ�SRV��UAV
		BUFFER_VIEW_TYPE ViewType = BUFFER_VIEW_UNDEFINED;

		// �ó�Աֻ���Formatted Buffer��Raw Buffer.
		BufferFormat Format;

		// ��View��ʾ������������Buffer�е�ƫ��
		UINT32 ByteOffset = 0;

		// ��View��ʾ������Ĵ�С
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
		// CPUDescriptorHeap��Descriptor��BufferView�������е���m_DescriptorHandle������ʱ���ͷ�Descriptor
		DescriptorHeapAllocation m_Descriptor;

		BufferViewDesc m_Desc;

		Buffer* const m_Buffer;
	};
}
