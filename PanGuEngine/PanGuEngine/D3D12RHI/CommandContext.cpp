#include "pch.h"
#include "CommandContext.h"
#include "CommandListManager.h"
#include "CommandQueue.h"
#include "RenderDevice.h"

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
		m_NumBarriersToFlush(0),
		m_DynamicGPUDescriptorAllocator(RenderDevice::GetSingleton().GetGPUDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), 128, "DynamicGPUDescriptorMgr")
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

		// TODO:重置渲染状态
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

		// TODO:重新设置一边渲染状态

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

		// 释放分配的Dynamic Descriptor
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

	void CommandContext::TransitionResource(GpuResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate /*= false*/)
	{
		D3D12_RESOURCE_STATES OldState = Resource.m_UsageState;

		// 限制Compute管线可以过度的状态
		if (m_Type == D3D12_COMMAND_LIST_TYPE_COMPUTE)
		{
			assert((OldState & VALID_COMPUTE_QUEUE_RESOURCE_STATES) == OldState);
			assert((NewState & VALID_COMPUTE_QUEUE_RESOURCE_STATES) == NewState);
		}

		if (OldState != NewState)
		{
			assert(m_NumBarriersToFlush < 16, "Exceeded arbitrary limit on buffered barriers");
			D3D12_RESOURCE_BARRIER& BarrierDesc = m_ResourceBarrierBuffer[m_NumBarriersToFlush++];

			BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			BarrierDesc.Transition.pResource = Resource.GetResource();
			BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			BarrierDesc.Transition.StateBefore = OldState;
			BarrierDesc.Transition.StateAfter = NewState;

			// Check to see if we already started the transition
			if (NewState == Resource.m_TransitioningState)
			{
				BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
				Resource.m_TransitioningState = (D3D12_RESOURCE_STATES)-1;
			}
			else
				BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

			Resource.m_UsageState = NewState;
		}

		if (FlushImmediate || m_NumBarriersToFlush == 16)
			FlushResourceBarriers();
	}

	// 使用Split Barrier来优化性能
	void CommandContext::BeginResourceTransition(GpuResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate /*= false*/)
	{
		// If it's already transitioning, finish that transition
		if (Resource.m_TransitioningState != (D3D12_RESOURCE_STATES)-1)
			TransitionResource(Resource, Resource.m_TransitioningState);

		D3D12_RESOURCE_STATES OldState = Resource.m_UsageState;

		if (OldState != NewState)
		{
			assert(m_NumBarriersToFlush < 16, "Exceeded arbitrary limit on buffered barriers");
			D3D12_RESOURCE_BARRIER& BarrierDesc = m_ResourceBarrierBuffer[m_NumBarriersToFlush++];

			BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			BarrierDesc.Transition.pResource = Resource.GetResource();
			BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			BarrierDesc.Transition.StateBefore = OldState;
			BarrierDesc.Transition.StateAfter = NewState;

			BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;

			Resource.m_TransitioningState = NewState;
		}

		if (FlushImmediate || m_NumBarriersToFlush == 16)
			FlushResourceBarriers();
	}

	// https://docs.microsoft.com/en-us/windows/win32/direct3d12/using-resource-barriers-to-synchronize-resource-states-in-direct3d-12
	// 文档中有说明：Applications should batch multiple transitions into one API call wherever possible. 
	// TODO: 所以这里尽量缓存到16个以后再一起执行,还不清楚原因
	void CommandContext::FlushResourceBarriers(void)
	{
		if (m_NumBarriersToFlush > 0)
		{
			m_CommandList->ResourceBarrier(m_NumBarriersToFlush, m_ResourceBarrierBuffer);
			m_NumBarriersToFlush = 0;
		}
	}

	void CommandContext::SetPipelineState(PipelineState* PSO)
	{
		// 绑定PSO
		m_CommandList->SetPipelineState(PSO->GetD3D12PipelineState());

		// 提交Static资源


	}

	// TODO: Remove dynamic_cast
	void GraphicsContext::ClearColor(GpuResourceDescriptor& RTV, D3D12_RECT* Rect /*= nullptr*/)
	{
		FlushResourceBarriers();

		const GpuResource* resource = RTV.GetResource();
		const GpuRenderTextureColor* rt = dynamic_cast<const GpuRenderTextureColor*>(resource);
		if(rt != nullptr)
			m_CommandList->ClearRenderTargetView(RTV.GetCpuHandle(), rt->GetClearColor().GetPtr(), (Rect == nullptr) ? 0 : 1, Rect);
	}

	void GraphicsContext::ClearColor(GpuResourceDescriptor& RTV, Color Colour, D3D12_RECT* Rect /*= nullptr*/)
	{
		FlushResourceBarriers();

		const GpuResource* resource = RTV.GetResource();
		const GpuRenderTextureColor* rt = dynamic_cast<const GpuRenderTextureColor*>(resource);
		if (rt != nullptr)
			m_CommandList->ClearRenderTargetView(RTV.GetCpuHandle(), Colour.GetPtr(), (Rect == nullptr) ? 0 : 1, Rect);
	}

	void GraphicsContext::ClearDepth(GpuResourceDescriptor& DSV)
	{
		FlushResourceBarriers();

		const GpuResource* resource = DSV.GetResource();
		const GpuRenderTextureDepth* depth = dynamic_cast<const GpuRenderTextureDepth*>(resource);
		if(depth != nullptr)
			m_CommandList->ClearDepthStencilView(DSV.GetCpuHandle(), D3D12_CLEAR_FLAG_DEPTH, depth->GetClearDepth(), depth->GetClearStencil(), 0, nullptr);
	}

	void GraphicsContext::ClearStencil(GpuResourceDescriptor& DSV)
	{
		FlushResourceBarriers();

		const GpuResource* resource = DSV.GetResource();
		const GpuRenderTextureDepth* depth = dynamic_cast<const GpuRenderTextureDepth*>(resource);
		if (depth != nullptr)
			m_CommandList->ClearDepthStencilView(DSV.GetCpuHandle(), D3D12_CLEAR_FLAG_STENCIL, depth->GetClearDepth(), depth->GetClearStencil(), 0, nullptr);
	}

	void GraphicsContext::ClearDepthAndStencil(GpuResourceDescriptor& DSV)
	{
		FlushResourceBarriers();

		const GpuResource* resource = DSV.GetResource();
		const GpuRenderTextureDepth* depth = dynamic_cast<const GpuRenderTextureDepth*>(resource);
		if (depth != nullptr)
			m_CommandList->ClearDepthStencilView(DSV.GetCpuHandle(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL , depth->GetClearDepth(), depth->GetClearStencil(), 0, nullptr);
	}

	void GraphicsContext::SetVertexBuffer(UINT Slot, const D3D12_VERTEX_BUFFER_VIEW& VBView)
	{
		m_CommandList->IASetVertexBuffers(Slot, 1, &VBView);
	}

	void GraphicsContext::SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& IBView)
	{
		m_CommandList->IASetIndexBuffer(&IBView);
	}

	void GraphicsContext::Draw(UINT VertexCount, UINT VertexStartOffset /*= 0*/)
	{
		DrawInstanced(VertexCount, 1, VertexStartOffset, 0);
	}

	void GraphicsContext::DrawIndexed(UINT IndexCount, UINT StartIndexLocation /*= 0*/, INT BaseVertexLocation /*= 0*/)
	{
		DrawIndexedInstanced(IndexCount, 1, StartIndexLocation, BaseVertexLocation, 0);
	}

	void GraphicsContext::DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation /*= 0*/, UINT StartInstanceLocation /*= 0*/)
	{
		FlushResourceBarriers();

		m_CommandList->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
	}

	void GraphicsContext::DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)
	{
		FlushResourceBarriers();

		m_CommandList->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
	}

}