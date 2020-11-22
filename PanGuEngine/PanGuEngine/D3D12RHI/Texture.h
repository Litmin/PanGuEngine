#pragma once
#include "D3D12ResourceBase.h"
#include "Buffer.h"
#include "TextureView.h"

namespace RHI 
{
    class RenderDevice;

    struct TextureDesc
    {
        std::wstring Name;

        RESOURCE_DIMENSION Type = RESOURCE_DIM_UNDEFINED;

        UINT32 Width;

        UINT32 Height;

        union
        {
            /// For a 1D array or 2D array, number of array slices
            UINT32 ArraySize = 1;

            /// For a 3D texture, number of depth slices
            UINT32 Depth;
        };

        DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN;

        UINT32 MipLevels = 1;

        UINT32 SampleCount = 1;

        USAGE Usage = USAGE_DEFAULT;

        // 可以使用BIND_SHADER_RESOURCE,BIND_RENDER_TARGET,BIND_DEPTH_STENCIL,BIND_UNORDERED_ACCESS
        BIND_FLAGS BindFlag = BIND_NONE;

        CPU_ACCESS_FLAGS CPUAccessFlags = CPU_ACCESS_NONE;

        MISC_TEXTURE_FLAGS MiscFlags = MISC_TEXTURE_FLAG_NONE;
        
        D3D12_CLEAR_VALUE ClearValue;
    };

    struct TextureSubResData
    {
        const void* Data = nullptr;

        // 可以使用Buffer中的数据初始化Texture
        Buffer* SrcBuffer = nullptr;

        // 使用Buffer初始化时，数据在Buffer中的偏移
        UINT32 SrcBufferOffset = 0;

        // 2D或3D纹理中每一行的数据大小（Byte）
        UINT32 Stride = 0;

        // 3D纹理中Depth Slice Stride
        UINT32 DepthStride = 0;
    };

    struct TextureData
    {
        TextureSubResData* SubResources    = nullptr;

        UINT32             NumSubresources = 0;
    };

    /**
    * Direct3D 12中把所有的GPU资源都用ID3D12Resource表示，因为所有资源本质上都是GPU中的一段内存，
    * 不同类型的资源用不同的Descriptor来描述
    */
    class Texture : D3D12ResourceBase
    {
    public:
        // 创建一个新的D3D12资源
        Texture(RenderDevice* renderDevice,
                const TextureDesc& texDesc,
                const TextureData* initData);

        // 使用已有的D3D12资源构造,例如SwapChain中的BackBuffer,SwapChain创建的Texture对象由SwapChain持有，所以生命周期跟SwapChain一致
        Texture(RenderDevice* renderDevice,
                const TextureDesc& texDesc,
                D3D12_RESOURCE_STATES initialState,
                ID3D12Resource* pTexture);

        ~Texture();

        // Texture View
        std::unique_ptr<TextureView> CreateView(const TextureViewDesc& viewDesc);
        TextureView* GetDefaultView(TEXTURE_VIEW_TYPE viewType);
        void CreateDefaultViews();

        // State
        void SetState(D3D12_RESOURCE_STATES State)
        {
            m_State = State;
        }

        D3D12_RESOURCE_STATES GetState() const
        {
            return this->m_State;
        }

        bool CheckState(D3D12_RESOURCE_STATES State) const
        {
            return (this->m_State & State) == State;
        }

        bool CheckAnyState(D3D12_RESOURCE_STATES States) const
        {
            return (this->m_State & States) != 0;
        }

        ID3D12Resource* GetD3D12Texture() { return GetD3D12Resource(); }

        D3D12_RESOURCE_DESC GetD3D12TextureDesc() const;

    protected:
        TextureDesc m_Desc;

        D3D12_RESOURCE_STATES m_State = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;

        std::unique_ptr<TextureView> m_defaultSRV;
        std::unique_ptr<TextureView> m_defaultRTV;
        std::unique_ptr<TextureView> m_defaultDSV;
        std::unique_ptr<TextureView> m_defaultUAV;
        
        RenderDevice* m_RenderDevice;
    };

}