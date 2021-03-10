#include "pch.h"
#include "CommandContext.h"
#include "CommandListManager.h"
#include "CommandQueue.h"

namespace RHI
{

	RHI::CommandContext* ContextManager::AllocateContext(D3D12_COMMAND_LIST_TYPE Type)
	{
		auto& AvailableContexts = m_AvailableContexts[Type];

		CommandContext* context = nullptr;
		if (AvailableContexts.empty())
		{
			context = new CommandContext(Type);
			m_ContextPool[Type].emplace_back(context);
			context->Initialize();
		}
		else
		{
			context = AvailableContexts.front();
			AvailableContexts.pop();
			context->Reset();
		}
		assert(context != nullptr);

		assert(context->m_Type == Type);

		return context;
	}

	void ContextManager::FreeContext(CommandContext* UsedContext)
	{
		assert(UsedContext != nullptr);
		m_AvailableContexts[UsedContext->m_Type].push(UsedContext);
	}

	CommandContext::CommandContext(D3D12_COMMAND_LIST_TYPE Type) :
		m_Type(Type),
		m_CommandList(nullptr),
		m_CurrentAllocator(nullptr),
		m_NumBarriersToFlush(0)
	{

	}

	void CommandContext::Initialize()
	{
		CommandListManager::GetSingleton().CreateNewCommandList(m_Type, m_CommandList.GetAddressOf(), &m_CurrentAllocator);
	}

	void CommandContext::Reset()
	{
		assert(m_CommandList != nullptr && m_CurrentAllocator == nullptr);

		m_CurrentAllocator = CommandListManager::GetSingleton().GetQueue(m_Type).RequestAllocator();
		m_CommandList->Reset(m_CurrentAllocator, nullptr);

		// TODO:ÖØÖÃäÖÈ¾×´Ì¬
	}

	CommandContext& CommandContext::Begin(const std::wstring ID /*= L""*/)
	{
		CommandContext* NewContext = ContextManager::GetSingleton().AllocateContext(D3D12_COMMAND_LIST_TYPE_DIRECT);
		NewContext->SetID(ID);
		return *NewContext;
	}

	uint64_t CommandContext::Flush(bool WaitForCompletion /*= false*/)
	{
		FlushResourceBarriers();

		assert(m_CurrentAllocator != nullptr);

		uint64_t FenceValue = CommandListManager::GetSingleton().GetQueue(m_Type).ExecuteCommandList(m_CommandList.Get());

		if (WaitForCompletion)
			CommandListManager::GetSingleton().WaitForFence(FenceValue, m_Type);

		m_CommandList->Reset(m_CurrentAllocator, nullptr);

		// TODO:ÖØÐÂÉèÖÃÒ»±ßäÖÈ¾×´Ì¬

		return FenceValue;
	}

	uint64_t CommandContext::Finish(bool WaitForCompletion /*= false*/)
	{
		assert(m_Type == D3D12_COMMAND_LIST_TYPE_DIRECT || m_Type == D3D12_COMMAND_LIST_TYPE_COMPUTE);

		FlushResourceBarriers();

		assert(m_CurrentAllocator != nullptr);

		CommandQueue& Queue = CommandListManager::GetSingleton().GetQueue(m_Type);

		uint64_t FenceValue = Queue.ExecuteCommandList(m_CommandList.Get());
		Queue.DiscardAllocator(FenceValue, m_CurrentAllocator);
		m_CurrentAllocator = nullptr;

		// ÊÍ·Å·ÖÅäµÄDynamic Descriptor
		m_DynamicGPUDescriptorAllocator->ReleaseAllocations();

		if (WaitForCompletion)
			CommandListManager::GetSingleton().WaitForFence(FenceValue, m_Type);

		ContextManager::GetSingleton().FreeContext(this);

		return FenceValue;
	}

	void CommandContext::InitializeBuffer(GpuBuffer& Dest, const void* Data, size_t NumBytes, size_t DestOffset /*= 0*/)
	{

	}

	void CommandContext::InitializeBuffer(GpuBuffer& Dest, const UploadBuffer& Src, size_t SrcOffset, size_t NumBytes /*= -1*/, size_t DestOffset /*= 0*/)
	{

	}

	void CommandContext::InitializeTexture(GpuResource& Dest, UINT NumSubresources, D3D12_SUBRESOURCE_DATA SubData[])
	{

	}

	DescriptorHeapAllocation CommandContext::AllocateDynamicGPUVisibleDescriptor(UINT Count /*= 1*/)
	{
		return m_DynamicGPUDescriptorAllocator->Allocate(Count);
	}

}