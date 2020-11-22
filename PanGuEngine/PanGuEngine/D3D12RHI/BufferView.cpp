#include "pch.h"
#include "BufferView.h"
#include "Buffer.h"
#include "RenderDevice.h"

namespace RHI
{
	BufferView::BufferView(RenderDevice*			  renderDevice, 
				     	   const BufferViewDesc&	  viewDesc,
						   Buffer*					  buffer, 
						   DescriptorHeapAllocation&& allocation) :
		m_Buffer{buffer},
		m_Desc{viewDesc},
		m_Descriptor{std::move(allocation)}
	{

	}
}

