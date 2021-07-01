#pragma once

#include "GpuRenderTexture.h"
#include "Core/Color.h"

namespace RHI 
{
    /**
    * 
    */
    class GpuRenderTextureCube : public GpuRenderTexture
    {
    public:
        GpuRenderTextureCube(UINT32 width, UINT32 height, UINT16 mipCount, DXGI_FORMAT format, Color clearColor);

        std::shared_ptr<GpuResourceDescriptor> CreateSRV();
        std::shared_ptr<GpuResourceDescriptor> CreateRTV(UINT8 faceIndex, UINT mipLevel);

    private:
        UINT16 m_MipLevels = 0;
        Color m_ClearColor;
    };

}