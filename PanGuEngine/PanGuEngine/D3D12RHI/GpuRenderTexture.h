#pragma once

#include "GpuTexture.h"

namespace RHI 
{
    /**
    * 
    */
    class GpuRenderTexture : public GpuTexture
    {
    public:
        GpuRenderTexture(UINT32 width, UINT32 height, D3D12_RESOURCE_DIMENSION dimension, DXGI_FORMAT format) :
            GpuTexture(width, height, dimension, format)
        {

        }

    };

}