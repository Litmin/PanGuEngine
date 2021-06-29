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

        GpuCubemap(UINT32 width, UINT32 height, D3D12_RESOURCE_DIMENSION dimension, DXGI_FORMAT format);

    protected:

        
        
    };

}