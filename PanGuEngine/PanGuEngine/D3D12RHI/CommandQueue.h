#pragma once
#include "CommandAllocatorPool.h"

namespace RHI 
{
    /*
    * 封装了CommandQueue和CommandAllocatorPool
    */
    class CommandQueue
    {
		friend class CommandListManager;
		friend class CommandContext;

    public:
		CommandQueue(D3D12_COMMAND_LIST_TYPE Type, ID3D12Device* Device);
		~CommandQueue();

		// 同步相关处理
		UINT64 IncrementFence(void);
		bool IsFenceComplete(UINT64 FenceValue);
		void StallForFence(UINT64 FenceValue, D3D12_COMMAND_LIST_TYPE Type);
		void StallForProducer(CommandQueue& Producer);
		void WaitForFence(UINT64 FenceValue);
		void WaitForIdle(void) { WaitForFence(IncrementFence()); }

		UINT64 GetNextFenceValue() const { return m_NextFenceValue; }
		UINT64 GetCompletedFenceValue() const { return m_pFence->GetCompletedValue(); }

		ID3D12CommandQueue* GetD3D12CommandQueue() { return m_CommandQueue.Get(); }

    private:
		UINT64 ExecuteCommandList(ID3D12CommandList* List);

		ID3D12CommandAllocator* RequestAllocator(void);
		void DiscardAllocator(uint64_t FenceValue, ID3D12CommandAllocator* Allocator);


		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;

		const D3D12_COMMAND_LIST_TYPE m_Type;
		CommandAllocatorPool m_AllocatorPool;

		Microsoft::WRL::ComPtr<ID3D12Fence> m_pFence;
		UINT64 m_NextFenceValue;
		UINT64 m_LastCompletedFenceValue;
		HANDLE m_FenceEventHandle;
    };

}