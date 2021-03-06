#include "pch.h"
#include "CommandContext.h"
#include "CommandListManager.h"
#include "CommandQueue.h"
#include "RenderDevice.h"
#include "GpuBuffer.h"

namespace RHI
{

	CommandContext* ContextManager::AllocateContext(D3D12_COMMAND_LIST_TYPE Type)
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
		m_DynamicGPUDescriptorAllocator(RenderDevice::GetSingleton().GetGPUDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), 128, "DynamicDescriptorMgr"),
		m_DynamicResourceHeap(RenderDevice::GetSingleton().GetDynamicResourceAllocator(), DYNAMIC_RESOURCE_PAGE_SIZE)
	{

	}

	CommandContext::~CommandContext()
	{
		// 释放分配的Dynamic Descriptor
		m_DynamicGPUDescriptorAllocator.ReleaseAllocations();
		// 释放分配的Dynamic Resource
		m_DynamicResourceHeap.ReleaseAllocatedPages();
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
		m_CurSRB = nullptr;
		m_CurPSO = nullptr;
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

		// TODO: CommandList Reset后，重新设置一遍渲染状态

		return FenceValue;
	}

	uint64_t CommandContext::Finish(bool WaitForCompletion /*= false*/, bool releaseDynamic /*= false*/)
	{
		assert(m_Type == D3D12_COMMAND_LIST_TYPE_DIRECT || m_Type == D3D12_COMMAND_LIST_TYPE_COMPUTE);

		FlushResourceBarriers();

		assert(m_CurrentAllocator != nullptr);

		CommandQueue& Queue = CommandListManager::GetSingleton().GetQueue(m_Type);

		// 清理Release Queue
		RenderDevice::GetSingleton().PurgeReleaseQueue(false);

		// 在每帧的末尾释放动态资源
		if (releaseDynamic)
		{
			// 释放分配的Dynamic Descriptor
			m_DynamicGPUDescriptorAllocator.ReleaseAllocations();

			// 释放分配的Dynamic Resource
			m_DynamicResourceHeap.ReleaseAllocatedPages();
		}
		
		uint64_t FenceValue = Queue.ExecuteCommandList(m_CommandList.Get());
		Queue.DiscardAllocator(FenceValue, m_CurrentAllocator);
		m_CurrentAllocator = nullptr;


		if (WaitForCompletion)
			CommandListManager::GetSingleton().WaitForFence(FenceValue, m_Type);

		ContextManager::GetSingleton().FreeContext(this);

		return FenceValue;
	}

	void CommandContext::InitializeBuffer(GpuBuffer& Dest, const void* Data, size_t NumBytes, size_t DestOffset /*= 0*/)
	{
		CommandContext& InitContext = CommandContext::Begin();

		// Copy到UploadBuffer,这里的UploadBuffer会自动释放，在析构函数中会调用SafeRelease
		GpuUploadBuffer uploadBuffer(1, NumBytes);
		void* dataPtr = uploadBuffer.Map();
		memcpy(dataPtr, Data, NumBytes);
		
		InitContext.TransitionResource(Dest, D3D12_RESOURCE_STATE_COPY_DEST, true);
		InitContext.m_CommandList->CopyBufferRegion(Dest.GetResource(), DestOffset, uploadBuffer.GetResource(), 0, NumBytes);
		InitContext.TransitionResource(Dest, D3D12_RESOURCE_STATE_GENERIC_READ, true);

		// Execute the command list and wait for it to finish so we can release the upload buffer
		InitContext.Finish(true);
	}

	void CommandContext::InitializeBuffer(GpuBuffer& Dest, const GpuUploadBuffer& Src, size_t SrcOffset, size_t NumBytes /*= -1*/, size_t DestOffset /*= 0*/)
	{
		CommandContext& InitContext = CommandContext::Begin();

		size_t MaxBytes = std::min<size_t>(Dest.GetBufferSize() - DestOffset, Src.GetBufferSize() - SrcOffset);
		NumBytes = std::min<size_t>(MaxBytes, NumBytes);

		InitContext.TransitionResource(Dest, D3D12_RESOURCE_STATE_COPY_DEST, true);
		InitContext.m_CommandList->CopyBufferRegion(Dest.GetResource(), DestOffset, (ID3D12Resource*)Src.GetResource(), SrcOffset, NumBytes);
		InitContext.TransitionResource(Dest, D3D12_RESOURCE_STATE_GENERIC_READ, true);

		// Execute the command list and wait for it to finish so we can release the upload buffer
		InitContext.Finish(true);
	}

	void CommandContext::InitializeTexture(GpuResource& Dest, UINT NumSubresources, D3D12_SUBRESOURCE_DATA SubData[])
	{
		CommandContext& InitContext = CommandContext::Begin();

		UINT64 uploadBufferSize = GetRequiredIntermediateSize(Dest.GetResource(), 0, NumSubresources);
		GpuUploadBuffer uploadBuffer(1, uploadBufferSize);

		UpdateSubresources(InitContext.m_CommandList.Get(), Dest.GetResource(), uploadBuffer.GetResource(), 0, 0, NumSubresources, SubData);
		InitContext.TransitionResource(Dest, D3D12_RESOURCE_STATE_GENERIC_READ);

		// Execute the command list and wait for it to finish so we can release the upload buffer
		InitContext.Finish(true);
	}

	DescriptorHeapAllocation CommandContext::AllocateDynamicGPUVisibleDescriptor(UINT Count /*= 1*/)
	{
		return m_DynamicGPUDescriptorAllocator.Allocate(Count);
	}

	D3D12DynamicAllocation CommandContext::AllocateDynamicSpace(size_t NumBytes, size_t Alignment)
	{
		return m_DynamicResourceHeap.Allocate(NumBytes, Alignment);
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
			assert(m_NumBarriersToFlush < 16 && "Exceeded arbitrary limit on buffered barriers");
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
			assert(m_NumBarriersToFlush < 16 && "Exceeded arbitrary limit on buffered barriers");
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
		assert(PSO != nullptr);

		m_CurPSO = PSO;
		m_CurSRB = nullptr;

		// 绑定PSO
		m_CommandList->SetPipelineState(PSO->GetD3D12PipelineState());
		m_CommandList->SetGraphicsRootSignature(PSO->GetD3D12RootSignature());


		// 提交Static资源
		PSO->CommitStaticSRB(*this);
	}

	void CommandContext::SetShaderResourceBinding(ShaderResourceBinding* SRB)
	{
		assert(SRB != nullptr);
		assert(m_CurPSO != nullptr);
		assert(SRB->m_PSO == m_CurPSO);

		if (m_CurSRB == SRB)
			return;
		// 记录SRB，在Draw之前提交Dynamic Shader Variable和Dynamic Buffer,因为Dynamic Buffer在修改数据时GPU地址会变，在Draw之前提交Dynamic Shader Vaiable减少更新频率
		m_CurSRB = SRB;

		m_CurPSO->CommitSRB(*this, SRB);
	}

	void CommandContext::CommitDynamic()
	{
		assert(m_CurPSO != nullptr);

		if (m_CurSRB == nullptr)
		{
			m_CurPSO->CommitDynamic(*this, nullptr);
		}
		else
		{
			if (m_CurSRB->m_PSO != m_CurPSO)
				LOG("aaa");
			assert(m_CurSRB != nullptr);
			assert(m_CurSRB->m_PSO == m_CurPSO);

			m_CurPSO->CommitDynamic(*this, m_CurSRB);
		}
	}

	void CommandContext::SetDescriptorHeap(ID3D12DescriptorHeap* cbvsrvuavHeap, ID3D12DescriptorHeap* samplerHeap)
	{
		ID3D12DescriptorHeap* descriptorHeaps[] = { cbvsrvuavHeap, samplerHeap };
		m_CommandList->SetDescriptorHeaps(2, descriptorHeaps);
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

	void GraphicsContext::SetViewport(const D3D12_VIEWPORT& vp)
	{
		m_CommandList->RSSetViewports(1, &vp);
	}

	void GraphicsContext::SetScissor(const D3D12_RECT& rect)
	{
		assert(rect.left < rect.right && rect.top < rect.bottom);
		m_CommandList->RSSetScissorRects(1, &rect);
	}

	void GraphicsContext::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY Topology)
	{
		m_CommandList->IASetPrimitiveTopology(Topology);
	}

	void GraphicsContext::SetRenderTargets(UINT NumRTVs, GpuResourceDescriptor* RTVs[], GpuResourceDescriptor* DSV /*= nullptr*/)
	{
		// 使用unique_ptr来管理动态数组的内存
		std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE[]> RTVHandles = nullptr;
		D3D12_CPU_DESCRIPTOR_HANDLE DSVHandle;

		if (NumRTVs > 0)
		{
			RTVHandles.reset(new D3D12_CPU_DESCRIPTOR_HANDLE[NumRTVs]);
			for (INT32 i = 0; i < NumRTVs; ++i)
			{
				RTVHandles[i] = RTVs[i]->GetCpuHandle();
			}
		}

		if (DSV != nullptr)
		{
			DSVHandle = DSV->GetCpuHandle();
			m_CommandList->OMSetRenderTargets(NumRTVs, RTVHandles.get(), FALSE, &DSVHandle);
		}
		else
		{
			m_CommandList->OMSetRenderTargets(NumRTVs, RTVHandles.get(), FALSE, nullptr);
		}
	}

	void GraphicsContext::SetVertexBuffer(UINT Slot, const D3D12_VERTEX_BUFFER_VIEW& VBView)
	{
		m_CommandList->IASetVertexBuffers(Slot, 1, &VBView);
	}

	void GraphicsContext::SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& IBView)
	{
		m_CommandList->IASetIndexBuffer(&IBView);
	}

	void GraphicsContext::SetConstantBuffer(UINT RootIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferAddress)
	{
		m_CommandList->SetGraphicsRootConstantBufferView(RootIndex, BufferAddress);
	}

	void GraphicsContext::SetDescriptorTable(UINT RootIndex, D3D12_GPU_DESCRIPTOR_HANDLE DescriptorTable)
	{
		m_CommandList->SetGraphicsRootDescriptorTable(RootIndex, DescriptorTable);
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

		// 提交动态资源
		CommitDynamic();

		m_CommandList->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
	}

	void GraphicsContext::DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)
	{
		FlushResourceBarriers();

		// 提交动态资源
		CommitDynamic();

		m_CommandList->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
	}

	// ComputeContext如果是异步计算，就在Compute Queue中分配
	ComputeContext& ComputeContext::Begin(const std::wstring& ID /*= L""*/, bool Async /*= false*/)
	{
		ComputeContext& NewContext = ContextManager::GetSingleton().AllocateContext(
			Async ? D3D12_COMMAND_LIST_TYPE_COMPUTE : D3D12_COMMAND_LIST_TYPE_DIRECT)->GetComputeContext();
		NewContext.SetID(ID);
		return NewContext;
	}

}