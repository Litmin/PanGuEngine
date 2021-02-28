#include "pch.h"
#include "CommandQueue.h"
#include "RenderDevice.h"
#include "CommandListManager.h"

using Microsoft::WRL::ComPtr;

namespace RHI
{
	CommandQueue::CommandQueue(D3D12_COMMAND_LIST_TYPE Type, ID3D12Device* Device) :
		m_Type(Type),
		m_AllocatorPool(Type, Device),
		m_NextFenceValue(1),
		m_LastCompletedFenceValue(0)
	{
		// 创建CommandQueue
		D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
		QueueDesc.Type = m_Type;
		QueueDesc.NodeMask = 1;
		ThrowIfFailed(Device->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&m_CommandQueue)));
		m_CommandQueue->SetName(L"CommandListManager::m_CommandQueue");

		// 创建Fence
		ThrowIfFailed(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence)));
		m_pFence->SetName(L"CommandListManager::m_pFence");

		// 创建Event
		m_FenceEventHandle = CreateEvent(nullptr, false, false, nullptr);
		assert(m_FenceEventHandle != NULL);
	}

	CommandQueue::~CommandQueue()
	{
		CloseHandle(m_FenceEventHandle);
	}

	uint64_t CommandQueue::IncrementFence(void)
	{
		m_CommandQueue->Signal(m_pFence.Get(), m_NextFenceValue);
		return m_NextFenceValue++;
	}

	bool CommandQueue::IsFenceComplete(uint64_t FenceValue)
	{
		// Avoid querying the fence value by testing against the last one seen.
		// The max() is to protect against an unlikely race condition that could cause the last
		// completed fence value to regress.
		if (FenceValue > m_LastCompletedFenceValue)
			m_LastCompletedFenceValue = std::max(m_LastCompletedFenceValue, m_pFence->GetCompletedValue());

		return FenceValue <= m_LastCompletedFenceValue;
	}


	void CommandQueue::StallForFence(uint64_t FenceValue, D3D12_COMMAND_LIST_TYPE Type)
	{
		// TODO: Get Command Queue
		CommandQueue& Producer = CommandListManager::GetSingleton().GetQueue((D3D12_COMMAND_LIST_TYPE)(FenceValue >> 56));
		m_CommandQueue->Wait(Producer.m_pFence.Get(), FenceValue);
	}

	void CommandQueue::StallForProducer(CommandQueue& Producer)
	{
		assert(Producer.m_NextFenceValue > 0);
		m_CommandQueue->Wait(Producer.m_pFence.Get(), Producer.m_NextFenceValue - 1);
	}

	void CommandQueue::WaitForFence(uint64_t FenceValue)
	{
		if (IsFenceComplete(FenceValue))
			return;

		m_pFence->SetEventOnCompletion(FenceValue, m_FenceEventHandle);
		WaitForSingleObject(m_FenceEventHandle, INFINITE);
		m_LastCompletedFenceValue = FenceValue;
	}

	uint64_t CommandQueue::ExecuteCommandList(ID3D12CommandList* List)
	{
		ThrowIfFailed(((ID3D12GraphicsCommandList*)List)->Close());

		// Kickoff the command list
		m_CommandQueue->ExecuteCommandLists(1, &List);

		// Signal the next fence value (with the GPU)
		m_CommandQueue->Signal(m_pFence.Get(), m_NextFenceValue);

		// And increment the fence value.  
		return m_NextFenceValue++;
	}

	ID3D12CommandAllocator* CommandQueue::RequestAllocator(void)
	{
		uint64_t CompletedFence = m_pFence->GetCompletedValue();

		return m_AllocatorPool.RequestAllocator(CompletedFence);
	}

	void CommandQueue::DiscardAllocator(uint64_t FenceValue, ID3D12CommandAllocator* Allocator)
	{
		m_AllocatorPool.DiscardAllocator(FenceValue, Allocator);
	}
}

