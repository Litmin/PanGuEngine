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
		uint64_t IncrementFence(void);
		bool IsFenceComplete(uint64_t FenceValue);
		void StallForFence(uint64_t FenceValue, D3D12_COMMAND_LIST_TYPE Type);
		void StallForProducer(CommandQueue& Producer);
		void WaitForFence(uint64_t FenceValue);
		void WaitForIdle(void) { WaitForFence(IncrementFence()); }

    private:
		uint64_t ExecuteCommandList(ID3D12CommandList* List);

		ID3D12CommandAllocator* RequestAllocator(void);
		void DiscardAllocator(uint64_t FenceValue, ID3D12CommandAllocator* Allocator);


		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;

		const D3D12_COMMAND_LIST_TYPE m_Type;
		CommandAllocatorPool m_AllocatorPool;

		Microsoft::WRL::ComPtr<ID3D12Fence> m_pFence;
		uint64_t m_NextFenceValue;
		uint64_t m_LastCompletedFenceValue;
		HANDLE m_FenceEventHandle;
    };

}