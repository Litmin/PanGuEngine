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

        virtual std::shared_ptr<GpuResourceDescriptor> CreateSRV() = 0;

    protected:

		UINT64 m_Width;
        UINT64 m_Height;
        D3D12_RESOURCE_DIMENSION m_Dimension;   // Dimension:Î¬¶È
        DXGI_FORMAT m_Format;
        
    };

}