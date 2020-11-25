#include "pch.h"
#include "CommandQueue.h"
#include "RenderDevice.h"

namespace RHI
{
	CommandQueue::CommandQueue(RenderDevice* renderDevice) :
		m_RenderDevice{renderDevice}
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		ThrowIfFailed(m_RenderDevice->GetD3D12Device()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_CmdQueue.GetAddressOf())));

		ThrowIfFailed(m_RenderDevice->GetD3D12Device()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));
	}

	CommandQueue::~CommandQueue()
	{
		CloseHandle(m_WaitForGPUEventHandle);
	}

	UINT64 CommandQueue::Submit(ID3D12GraphicsCommandList* commandList)
	{
		UINT64 fenceValue = m_NextFenceValue;

		m_NextFenceValue++;

		if (commandList != nullptr)
		{
			ID3D12CommandList* const ppCmdLists[] = { commandList };
			m_CmdQueue->ExecuteCommandLists(1, ppCmdLists);
		}

		m_CmdQueue->Signal(m_Fence.Get(), fenceValue);

		return fenceValue;
	}

	void CommandQueue::SignalFence(ID3D12Fence* pFence, UINT64 Value)
	{
		m_CmdQueue->Signal(pFence, Value);
	}

	UINT64 CommandQueue::FlushCommandQueue()
	{
		UINT64 LastSignaledFenceValue = m_NextFenceValue;

		m_NextFenceValue++;

		m_CmdQueue->Signal(m_Fence.Get(), LastSignaledFenceValue);

		if (GetCompletedFenceValue() < LastSignaledFenceValue)
		{
			m_Fence->SetEventOnCompletion(LastSignaledFenceValue, m_WaitForGPUEventHandle);
			WaitForSingleObject(m_WaitForGPUEventHandle, INFINITE);

			assert(GetCompletedFenceValue() == LastSignaledFenceValue && "Unexpected signaled fence value");
		}

		return LastSignaledFenceValue;
	}

	UINT64 CommandQueue::GetCompletedFenceValue()
	{
		UINT64 CompletedFenceValue = m_Fence->GetCompletedValue();
		if (CompletedFenceValue > m_LastCompletedFenceValue)
			m_LastCompletedFenceValue = CompletedFenceValue;
		return m_LastCompletedFenceValue;
	}
}

