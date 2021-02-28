#pragma once
#include "DescriptorHeap.h"
#include "PipelineState.h"

namespace RHI 
{
	class CommandContext;
	class GraphicContext;
	class ComputeContext;

	// TODO:该功能移动到RenderDevice中
	class ContextManager : public Singleton<ContextManager>
	{
	public:
		CommandContext* AllocateContext(D3D12_COMMAND_LIST_TYPE Type);
		void FreeContext(CommandContext* UsedContext);

	private:
		std::vector<std::unique_ptr<CommandContext> > m_ContextPool[4];
		std::queue<CommandContext*> m_AvailableContexts[4];
	};

	struct NonCopyable
	{
		NonCopyable() = default;
		NonCopyable(const NonCopyable&) = delete;
		NonCopyable& operator=(const NonCopyable&) = delete;
	};

    /**
    * CommandList和CommandListAllocator的集合体
	* 调用Static函数Begin会请求一个CommandContext，然后可以开始录制命令，调用End把命令推入CommandQueue
	* Begin时还会请求一个新的Allocator，End时回收这个Allocator
	* 每个线程使用自己的CommandContext
    */
    class CommandContext : public NonCopyable
    {
		friend ContextManager;

		// CommandContext由ContextManager创建,所以把构造函数的访问权限设为private
	private:
		CommandContext(D3D12_COMMAND_LIST_TYPE Type);
		// 创建CommandContext时调用，该函数会创建一个CommandList，并请求一个Allocator
		void Initialize();
		// 复用CommandContext时调用，重置渲染状态，该函数会请求一个Allocator，并调用CommandList::Reset
		void Reset();

	public:
		// 开始记录命令
		 static CommandContext& Begin(const std::wstring ID = L"");
		// Flush existing commands to the GPU but keep the context alive
		uint64_t Flush(bool WaitForCompletion = false);
		// 完成记录命令
		uint64_t Finish(bool WaitForCompletion = false);


		// Graphic Context可以转换成Compute Context，但是Compute Context不能转换成Graphic Context
		GraphicsContext& GetGraphicsContext() 
		{
			assert(m_Type != D3D12_COMMAND_LIST_TYPE_COMPUTE, "Cannot convert async compute context to graphics");
			return reinterpret_cast<GraphicsContext&>(*this);
		}

		ComputeContext& GetComputeContext() 
		{
			return reinterpret_cast<ComputeContext&>(*this);
		}

		// Resource Barrier


		// 渲染状态和资源绑定
		void SetPipelineState();


	private:
		void SetID(const std::wstring& ID) { m_ID = ID; }

		D3D12_COMMAND_LIST_TYPE m_Type;
		// CommandList由CommandContext持有，CommandAllocator由对象池管理
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;
		ID3D12CommandAllocator* m_CurrentAllocator;

		// ResourceBarrier到了16个时Flush
		D3D12_RESOURCE_BARRIER m_ResourceBarrierBuffer[16];
		UINT m_NumBarriersToFlush;

		// TODO:Descriptor Heap

		// Dynamic Descriptor
		// Dynamic Constant Buffer

		std::wstring m_ID;
    };

    class GraphicsContext : public CommandContext
    {
    public:

		static GraphicsContext& Begin(const std::wstring& ID = L"")
		{
			return CommandContext::Begin(ID).GetGraphicsContext();
		}


		void Draw(UINT VertexCount, UINT VertexStartOffset = 0);
		void DrawIndexed(UINT IndexCount, UINT StartIndexLocation = 0, INT BaseVertexLocation = 0);
		void DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount,
			UINT StartVertexLocation = 0, UINT StartInstanceLocation = 0);
		void DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation,
			INT BaseVertexLocation, UINT StartInstanceLocation);
		// TODO: Indirect Draw
    };

	// TODO:Compute
    class ComputeContext : public CommandContext
    {
	public:


    };
}