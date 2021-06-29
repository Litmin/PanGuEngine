#pragma once

#include "GpuTexture.h"

namespace RHI 
{

    /**
    * 
    */
    class GpuCubemap : public GpuTexture
    {
    public:

        GpuCubemap(UINT32 width, UINT32 height, UINT16 mipCount, DXGI_FORMAT format, D3D12_SUBRESOURCE_DATA* initData);

        std::shared_ptr<GpuResourceDescriptor> CreateSRV();

    protected:

        UINT16 m_MipLevels = 0;
        
    };

}