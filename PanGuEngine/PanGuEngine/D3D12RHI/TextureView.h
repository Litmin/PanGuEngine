#pragma once
#include "DescriptorHeap.h"
#include "Sampler.h"
#include "IShaderResource.h"

namespace RHI 
{
    class RenderDevice;
    class Texture;

    struct TextureViewDesc
    {
        TEXTURE_VIEW_TYPE ViewType = TEXTURE_VIEW_UNDEFINED;

        RESOURCE_DIMENSION TextureDim = RESOURCE_DIM_UNDEFINED;

        DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN;

        UINT32 MostDetailedMip = 0;

        UINT32 NumMipLevels = 0;

        union
        {
            /// For a texture array, first array slice to address in the view
            UINT32 FirstArraySlice = 0;

            /// For a 3D texture, first depth slice to address the view
            UINT32 FirstDepthSlice;
        };

        union
        {
            /// For a texture array, number of array slices to address in the view.
            /// Set to 0 to address all array slices.
            UINT32 NumArraySlices = 0;

            /// For a 3D texture, number of depth slices to address in the view
            /// Set to 0 to address all depth slices.
            UINT32 NumDepthSlices;
        };

        UAV_ACCESS_FLAG AccessFlags = UAV_ACCESS_UNSPECIFIED;

        TEXTURE_VIEW_FLAGS Flags = TEXTURE_VIEW_FLAG_NONE;
    };

    /**
    * 
    */
    class TextureView : public IShaderResource
    {
    public:
        TextureView(RenderDevice* renderDevice,
                    const TextureViewDesc& textureViewDesc,
                    Texture* texture,
                    DescriptorHeapAllocation&& allocation);

        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle()
        {
            return m_Descriptor.GetCpuHandle();
        }

        Texture* GetTexture() const
        {
            return m_Texture;
        }

        void SetSampler(std::unique_ptr<Sampler> sampler)
        {
            m_Sampler = std::move(sampler);
        }

        Sampler* GetSampler() const
        {
            return m_Sampler.get();
        }

    protected:
        TextureViewDesc m_Desc;

        DescriptorHeapAllocation m_Descriptor;

        Texture* const m_Texture;

        std::unique_ptr<Sampler> m_Sampler;

        RenderDevice* m_RenderDevice;
    };

}