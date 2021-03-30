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

        // 从Swapchain的资源创建
        GpuRenderTextureColor(ID3D12Resource* resource, D3D12_RESOURCE_DESC desc, Color clearColor = Color(0.0f, 0.0f, 0.0f, 0.0f));

        GpuRenderTextureColor(UINT32 width, UINT32 height, 
                              D3D12_RESOURCE_DIMENSION dimension, 
                              DXGI_FORMAT format, 
                              Color clearColor = Color(0.0f, 0.0f, 0.0f, 0.0f));
        

        std::shared_ptr<GpuResourceDescriptor> CreateSRV();
        std::shared_ptr<GpuResourceDescriptor> CreateRTV();
        std::shared_ptr<GpuResourceDescriptor> CreateUAV();

        Color GetClearColor() const { return m_ClearColor; }

    protected:

        Color m_ClearColor;
    };

}