#pragma once

namespace RHI 
{
    class RenderDevice;

    struct CommandQueueDesc
    {

    };

    class CommandQueue
    {
    public:
        CommandQueue(RenderDevice* renderDevice);
        ~CommandQueue();

        // 把CommandList记录的命令提交到CommandQueue,返回跟CommandList关联的Fence Value
        UINT64 Submit(ID3D12GraphicsCommandList* commandList);

        // Signal指定的Fence
        void SignalFence(ID3D12Fence* pFence, UINT64 Value);

        // 等待该CommandQueue中的命令全部执行完成
        UINT64 FlushCommandQueue();

        ID3D12CommandQueue* GetD3D12CommandQueue() { return m_CmdQueue.Get(); }

        // 获取下一个要Signal的Fence Value
        UINT64 GetNextFenceValue() const { return m_NextFenceValue; }

        // 获取该CommandQueue完成的Fence Value
        UINT64 GetCompletedFenceValue();

    private:
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CmdQueue;

        Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;

        RenderDevice* m_RenderDevice;

        UINT64 m_NextFenceValue = 1;
        UINT64 m_LastCompletedFenceValue = 0;
        HANDLE m_WaitForGPUEventHandle = {};
    };

}