#pragma once

#include "GpuResource.h"

namespace RHI 
{

    class GpuResourceDescriptor;

    /**
    * 
    */
    // TODO: MipMap
    class GpuTexture : public GpuResource
    {
    public:

        GpuTexture(UINT32 width, UINT32 height, D3D12_RESOURCE_DIMENSION dimension, DXGI_FORMAT format) :
            m_Width(width),
            m_Height(height),
            m_Dimension(dimension),
            m_Format(format)
        {
        }

    protected:

        // 格式转换
        // 转换成TypeLess
		static DXGI_FORMAT GetBaseFormat(DXGI_FORMAT Format);
		static DXGI_FORMAT GetUAVFormat(DXGI_FORMAT Format);
		static DXGI_FORMAT GetDSVFormat(DXGI_FORMAT Format);
		static DXGI_FORMAT GetDepthFormat(DXGI_FORMAT Format);
		static DXGI_FORMAT GetStencilFormat(DXGI_FORMAT Format);
		static size_t BytesPerPixel(DXGI_FORMAT Format);

		UINT64 m_Width;
        UINT64 m_Height;
        D3D12_RESOURCE_DIMENSION m_Dimension;   // Dimension:维度
        DXGI_FORMAT m_Format;
        
    };

}