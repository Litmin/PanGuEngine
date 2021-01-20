#pragma once

#include "BufferView.h"
#include "DeviceContext.h"
#include "D3D12ResourceBase.h"

namespace RHI
{
    class RenderDevice;

    /**
    * [D3D11_BUFFER_DESC]https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_buffer_desc
    */

    struct BufferDesc
    {
        const std::wstring Name;

        // Buffer�Ĵ�С��DX12��Constant Buffer�Ĵ�С������256�ֽڶ��룡
        UINT32 SizeInBytes = 0;

        // ��������Render Target��Depth Stencil
        BIND_FLAGS BindFlags = BIND_NONE;

        USAGE Usage = USAGE_DEFAULT;

        CPU_ACCESS_FLAGS CPUAccessFlags = CPU_ACCESS_NONE;

        BUFFER_MODE Mode = BUFFER_MODE_UNDEFINED;

        UINT32 ElementByteStride = 0;
    };

    struct BufferData
    {
        const void* pData = nullptr;

        UINT32 DataSize = 0;
    };

    /**
    * Buffer��ʵ��
    */
    class Buffer : public D3D12ResourceBase
    {
    public:
        // �������캯����һ��ʹ�����ݳ�ʼ������һ��ʹ��D3D12Resource��ResourceState��ʼ��
        Buffer(RenderDevice* pRenderDevice, 
               const BufferDesc& desc, 
               const BufferData* pBufferData = nullptr);

        Buffer(RenderDevice* pRenderDevice, 
               const BufferDesc& desc, 
               D3D12_RESOURCE_STATES initialState, 
               ID3D12Resource* pD3D12Buffer);

        // �����������ͷ�Buffer��Դ
        virtual ~Buffer() override;

        // Buffer View
        std::unique_ptr<BufferView> CreateView(const BufferViewDesc& viewDesc);
        BufferView* GetDefaultView(BUFFER_VIEW_TYPE viewType);
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

        const BufferDesc& GetDesc() const { return m_Desc; };

        // ��Vertex Buffer��Index Bufferʱ����
        ID3D12Resource* GetD3D12Buffer(UINT64& DataStartByteOffset, DeviceContext* pContext);
        // ֱ����ΪRoot CBV��ʱ����
        D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(UINT32 contextId, DeviceContext* pContext);

    protected:
        BufferDesc m_Desc;

        // ��Դ�ĵ�ǰ״̬
        D3D12_RESOURCE_STATES m_State = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
        // Buffer��Ĭ��Descriptor��Buffer���У��������ڸ�Bufferһ��
        std::unique_ptr<BufferView> m_DefaultCBV;
        std::unique_ptr<BufferView> m_DefaultSRV;
        std::unique_ptr<BufferView> m_DefaultUAV;

        RenderDevice* m_RenderDevice;
    };
}





