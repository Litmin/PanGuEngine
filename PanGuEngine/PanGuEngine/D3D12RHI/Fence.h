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

        // ��ȡGPU��ɵĸ�Fence��ֵ
        UINT64 GetCompletedValue();

        // ��Fence����ΪValue
        void Reset(UINT64 Value);

        // �ȴ�GPUִ�е�Value
        void WaitForCompletion(UINT64 Value);

        ID3D12Fence* GetD3D12Fence() { return m_Fence.Get(); }

    private:
        Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
    };

}