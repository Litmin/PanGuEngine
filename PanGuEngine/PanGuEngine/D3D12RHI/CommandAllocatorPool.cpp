#include "pch.h"
#include "CommandAllocatorPool.h"


namespace RHI
{
	CommandAllocatorPool::CommandAllocatorPool(D3D12_COMMAND_LIST_TYPE Type, ID3D12Device* Device) :
		m_cCommandListType(Type),
		m_Device(Device)
	{

	}

	// ����һ��CommandAllocator����������еĵ�һ��Allocator�Ƿ���ã�
	// ���CompletedFenceValue > Pool�е�FenceValue������GPU�Ѿ���������Allocator�������ʾ����
	// �������´���һ��CommandAllocator
	ID3D12CommandAllocator* CommandAllocatorPool::RequestAllocator(UINT64 CompletedFenceValue)
	{
		ID3D12CommandAllocator* pAllocator = nullptr;

		if (!m_ReadyAllocators.empty())
		{
			std::pair<uint64_t, ID3D12CommandAllocator*>& AllocatorPair = m_ReadyAllocators.front();

			if (AllocatorPair.first <= CompletedFenceValue)
			{
				pAllocator = AllocatorPair.second;
				ThrowIfFailed(pAllocator->Reset());
				m_ReadyAllocators.pop();
			}
		}

		// If no allocator's were ready to be reused, create a new one
		if (pAllocator == nullptr)
		{
			ThrowIfFailed(m_Device->CreateCommandAllocator(m_cCommandListType, IID_PPV_ARGS(&pAllocator)));
			wchar_t AllocatorName[32];
			swprintf(AllocatorName, 32, L"CommandAllocator %zu", m_Allocators.size());
			pAllocator->SetName(AllocatorName);
			m_Allocators.push_back(pAllocator);
		}

		return pAllocator;
	}

	void CommandAllocatorPool::DiscardAllocator(uint64_t FenceValue, ID3D12CommandAllocator* Allocator)
	{
		m_ReadyAllocators.push(std::make_pair(FenceValue, Allocator));
	}

}