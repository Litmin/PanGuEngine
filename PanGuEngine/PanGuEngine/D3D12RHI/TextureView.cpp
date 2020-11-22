#include "pch.h"
#include "TextureView.h"

namespace RHI
{
	TextureView::TextureView(RenderDevice* renderDevice, 
							 const TextureViewDesc& textureViewDesc, 
							 Texture* texture, 
							 DescriptorHeapAllocation&& allocation) :
		m_RenderDevice{renderDevice},
		m_Desc{textureViewDesc},
		m_Texture{texture},
		m_Descriptor{std::move(allocation)}
	{
	}
}

