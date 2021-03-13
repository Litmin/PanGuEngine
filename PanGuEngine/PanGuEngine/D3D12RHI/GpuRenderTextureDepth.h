#pragma once

#include "GpuRenderTexture.h"

namespace RHI 
{
    /**
    * 
    */
    class GpuRenderTextureDepth : public GpuRenderTexture
    {
    public:
        GpuRenderTextureDepth(UINT32 width, UINT32 height, DXGI_FORMAT format);

        std::shared_ptr<GpuResourceDescriptor> CreateDSV();
        std::shared_ptr<GpuResourceDescriptor> CreateDepthSRV();
        std::shared_ptr<GpuResourceDescriptor> CreateStencilSRV();

    };

}