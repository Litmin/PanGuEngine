#pragma once

#include "GpuRenderTexture.h"
#include "Core/Color.h"

namespace RHI 
{
    /**
    * 
    */
    class GpuRenderTextureColor : public GpuRenderTexture
    {
    public:
        GpuRenderTextureColor(UINT32 width, UINT32 height, 
                              D3D12_RESOURCE_DIMENSION dimension, 
                              DXGI_FORMAT format, 
                              Color clearColor = Color(0.0f, 0.0f, 0.0f, 0.0f));
        

        std::shared_ptr<GpuResourceDescriptor> CreateSRV();
        std::shared_ptr<GpuResourceDescriptor> CreateRTV();
        std::shared_ptr<GpuResourceDescriptor> CreateUAV();

    protected:

        Color m_ClearColor;
    };

}