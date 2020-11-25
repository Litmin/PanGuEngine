#pragma once
#include "D3D12DeviceObject.h"

namespace RHI 
{
    class RenderDevice;

    class Fence : public D3D12DeviceObject
    {
    public:
        Fence(RenderDevice* renderDevice);
        virtual ~Fence();

        // 获取GPU完成的该Fence的值
        UINT64 GetCompletedValue();

        // 把Fence重置为Value
        void Reset(UINT64 Value);

        // 等待GPU执行到Value
        void WaitForCompletion(UINT64 Value);

        ID3D12Fence* GetD3D12Fence() { return m_Fence.Get(); }

    private:
        Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
    };

}