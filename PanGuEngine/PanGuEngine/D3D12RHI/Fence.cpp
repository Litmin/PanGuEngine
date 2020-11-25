#include "pch.h"
#include "Fence.h"
#include "RenderDevice.h"

namespace RHI
{
    Fence::Fence(RenderDevice* renderDevice)
    {
        ThrowIfFailed(renderDevice->GetD3D12Device()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));
    }

    Fence::~Fence()
    {
    }

    UINT64 Fence::GetCompletedValue()
    {
        return m_Fence->GetCompletedValue();
    }

    void Fence::Reset(UINT64 Value)
    {
        m_Fence->Signal(Value);

    }

    void Fence::WaitForCompletion(UINT64 Value)
    {
        if (m_Fence->GetCompletedValue() < Value)
        {
            HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

            ThrowIfFailed(m_Fence->SetEventOnCompletion(Value, eventHandle));

            WaitForSingleObject(eventHandle, INFINITE);
            CloseHandle(eventHandle);
        }
    }
}

