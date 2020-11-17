#pragma once

#include "BufferView.h"
#include "RenderDevice.h"
#include "DeviceContext.h"
#include "D3D12ResourceBase.h"

namespace RHI
{
    /**
    * [D3D11_BUFFER_DESC]https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_buffer_desc
    */

    struct BufferDesc
    {
        // Buffer的大小，DX12的Constant Buffer的大小必须是256字节对齐！
        UINT32 uiSizeInBytes = 0;

        // 不能用作Render Target和Depth Stencil
        BIND_FLAGS BindFlags = BIND_FLAGS::BIND_NONE;

        USAGE Usage = USAGE::USAGE_DEFAULT;

        CPU_ACCESS_FLAGS CPUAccessFlags = CPU_ACCESS_FLAGS::CPU_ACCESS_NONE;

        BUFFER_MODE Mode = BUFFER_MODE::BUFFER_MODE_UNDEFINED;

        UINT32 ElementByteStride = 0;
    };

    struct BufferData
    {
        const void* pData = nullptr;

        UINT32 DataSize = 0;
    };

    /**
    * Buffer的实现
    */
    class Buffer : public D3D12ResourceBase
    {
    public:
        // 两个构造函数，一个使用数据初始化，另一个使用D3D12Resource和ResourceState初始化
        Buffer(RenderDevice* pRenderDevice, 
               const BufferDesc& desc, 
               const BufferData* pBufferData = nullptr);

        Buffer(RenderDevice* pRenderDevice, 
               const BufferDesc& desc, 
               D3D12_RESOURCE_STATES initialState, 
               ID3D12Resource* pD3D12Buffer);

        //void CreateView(const BufferViewDesc& viewDesc, BufferView** ppView);
        BufferView* GetDefaultView(BUFFER_VIEW_TYPE viewType);
        void CreateDefaultViews();


        ID3D12Resource* GetD3D12Buffer(UINT64& DataStartByteOffset, DeviceContext* pContext);

        void SetD3D12ResourceState(D3D12_RESOURCE_STATES state);
        D3D12_RESOURCE_STATES GetD3D12ResourceState() const;

        D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(UINT32 contextId, DeviceContext* pContext);
        //D3D12_CPU_DESCRIPTOR_HANDLE GetCBVHandle() { return m_CBVDescriptorAllocation.GetCpuHandle(); }

    protected:


        D3D12_RESOURCE_STATES m_State = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
        std::unique_ptr<BufferView> m_DefaultSRV;
        std::unique_ptr<BufferView> m_DefaultUAV;
    };
}





