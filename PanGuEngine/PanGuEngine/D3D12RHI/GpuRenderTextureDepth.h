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
        GpuRenderTextureDepth(UINT32 width, UINT32 height, DXGI_FORMAT format, float clearDepth = 1.0f, UINT8 clearStencil = 0);

        std::shared_ptr<GpuResourceDescriptor> CreateDSV();
        std::shared_ptr<GpuResourceDescriptor> CreateDepthSRV();
        std::shared_ptr<GpuResourceDescriptor> CreateStencilSRV();

        float GetClearDepth() const { return m_ClearDepth; }
        UINT8 GetClearStencil() const { return m_ClearStencil; }

    protected:
        float m_ClearDepth;
		UINT8 m_ClearStencil;

    };

}