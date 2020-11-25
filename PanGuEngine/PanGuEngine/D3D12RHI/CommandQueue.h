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

        // ��CommandList��¼�������ύ��CommandQueue,���ظ�CommandList������Fence Value
        UINT64 Submit(ID3D12GraphicsCommandList* commandList);

        // Signalָ����Fence
        void SignalFence(ID3D12Fence* pFence, UINT64 Value);

        // �ȴ���CommandQueue�е�����ȫ��ִ�����
        UINT64 FlushCommandQueue();

        ID3D12CommandQueue* GetD3D12CommandQueue() { return m_CmdQueue.Get(); }

        // ��ȡ��һ��ҪSignal��Fence Value
        UINT64 GetNextFenceValue() const { return m_NextFenceValue; }

        // ��ȡ��CommandQueue��ɵ�Fence Value
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