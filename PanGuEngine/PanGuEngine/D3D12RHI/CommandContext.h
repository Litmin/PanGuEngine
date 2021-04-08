#pragma once
#include "DescriptorHeap.h"
#include "PipelineState.h"
#include "GpuBuffer.h"
#include "GpuTexture.h"
#include "DescriptorHeap.h"
#include "GpuRenderTextureColor.h"
#include "GpuRenderTextureDepth.h"
#include "DynamicResource.h"

namespace RHI 
{
	class CommandContext;
	class GraphicsContext;
	class ComputeContext;


	// Compute Command List只支持这几种状态的过渡
	#define VALID_COMPUTE_QUEUE_RESOURCE_STATES \
	    ( D3D12_RESOURCE_STATE_UNORDERED_ACCESS \
	    | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE \
		| D3D12_RESOURCE_STATE_COPY_DEST \
		| D3D12_RESOURCE_STATE_COPY_SOURCE )


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
			assert(m_Type != D3D12_COMMAND_LIST_TYPE_COMPUTE && "Cannot convert async compute context to graphics");
			return reinterpret_cast<GraphicsContext&>(*this);
		}

		ComputeContext& GetComputeContext() 
		{
			return reinterpret_cast<ComputeContext&>(*this);
		}


		// TODO: Implement 初始化资源
		static void InitializeBuffer(GpuBuffer& Dest, const void* Data, size_t NumBytes, size_t DestOffset = 0);
		static void InitializeBuffer(GpuBuffer& Dest, const GpuUploadBuffer& Src, size_t SrcOffset, size_t NumBytes = -1, size_t DestOffset = 0);
		static void InitializeTexture(GpuResource& Dest, UINT NumSubresources, D3D12_SUBRESOURCE_DATA SubData[]);


		// Dynamic Descriptor在GPUDescriptorHeap上分配,在Finish中释放
		DescriptorHeapAllocation AllocateDynamicGPUVisibleDescriptor(UINT Count = 1);

		// 为Dynamic Resource分配内存
		D3D12DynamicAllocation AllocateDynamicSpace(size_t NumBytes, size_t Alignment);

		/* Resource Barrier TODO: UAVBarrier
		* GpuResource有两个成员：m_UsageState、m_TransitioningState，
		* m_UsageState:表示资源当前的状态，调用TransitionResource，会使用m_UsageState检查资源的当前状态和过度之后的状态，如果不相等才提交Resource Barrier
		* m_TransitioningState:表示资源正在过度的状态，用在Split Barrier，标记资源已经进行了Begin Barrier
		* BeginResourceTransition使用Split Barrier来优化性能
		* https://docs.microsoft.com/en-us/windows/win32/direct3d12/using-resource-barriers-to-synchronize-resource-states-in-direct3d-12
		* 文档中有说明：Applications should batch multiple transitions into one API call wherever possible.
		* TODO: 所以这里尽量缓存到16个以后再一起执行(FlushResourceBarriers),还不清楚原因
		*/
		void TransitionResource(GpuResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate = false);
		void BeginResourceTransition(GpuResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate = false);
		inline void FlushResourceBarriers(void);


		/*渲染状态和资源绑定
		* 切换PSO时，会自动提交Static SRB，Static SRB只绑定Static Shader Variable
		* 切换SRB时，会自动提交Mutable SRB的资源
		* Draw Call前，会自动提交当前SRB中的Dynamic Shader Variable和Dynamic Buffer，提交前会检查是否资源是否有更新
		*/
		void SetPipelineState(PipelineState* PSO);
		void SetShaderResourceBinding(ShaderResourceBinding* SRB);
		void CommitDynamic();	// TODO: Make private

		void SetDescriptorHeap(ID3D12DescriptorHeap* cbvsrvuavHeap, ID3D12DescriptorHeap* samplerHeap);

	protected:
		void SetID(const std::wstring& ID) { m_ID = ID; }

		D3D12_COMMAND_LIST_TYPE m_Type;
		// CommandList由CommandContext持有，CommandAllocator由对象池管理
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;
		ID3D12CommandAllocator* m_CurrentAllocator;

		// ResourceBarrier到了16个时Flush
		D3D12_RESOURCE_BARRIER m_ResourceBarrierBuffer[16];
		UINT m_NumBarriersToFlush;

		// Dynamic Descriptor
		DynamicSuballocationsManager m_DynamicGPUDescriptorAllocator;

		// Dynamic Resource
		DynamicResourceHeap m_DynamicResourceHeap;

		// 资源绑定
		PipelineState* m_CurPSO = nullptr;
		ShaderResourceBinding* m_CurSRB = nullptr;


		std::wstring m_ID;
    };

    class GraphicsContext : public CommandContext
    {
    public:
		static GraphicsContext& Begin(const std::wstring& ID = L"")
		{
			return CommandContext::Begin(ID).GetGraphicsContext();
		}

		// Clear
		void ClearColor(GpuResourceDescriptor& RTV, D3D12_RECT* Rect = nullptr);
		void ClearColor(GpuResourceDescriptor& RTV, Color Colour, D3D12_RECT* Rect = nullptr);
		void ClearDepth(GpuResourceDescriptor& DSV);
		void ClearStencil(GpuResourceDescriptor& DSV);
		void ClearDepthAndStencil(GpuResourceDescriptor& DSV);

		void SetViewport(const D3D12_VIEWPORT& vp);
		void SetScissor(const D3D12_RECT& rect);
		void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY Topology);

		void SetRenderTargets(UINT NumRTVs, GpuResourceDescriptor* RTVs[], GpuResourceDescriptor* DSV = nullptr);

		// Vertex Buffer、Index Buffer
		void SetVertexBuffer(UINT Slot, const D3D12_VERTEX_BUFFER_VIEW& VBView);
		void SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& IBView);

		// Constant Buffer
		void SetConstantBuffer(UINT RootIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferAddress);

		// Descriptor
		void SetDescriptorTable(UINT RootIndex, D3D12_GPU_DESCRIPTOR_HANDLE DescriptorTable);

		void Draw(UINT VertexCount, UINT VertexStartOffset = 0);
		void DrawIndexed(UINT IndexCount, UINT StartIndexLocation = 0, INT BaseVertexLocation = 0);
		void DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount,
			UINT StartVertexLocation = 0, UINT StartInstanceLocation = 0);
		void DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation,
			INT BaseVertexLocation, UINT StartInstanceLocation);
		// TODO: Indirect Draw

	protected:

    };

	// TODO: Compute
    class ComputeContext : public CommandContext
    {
	public:
		static ComputeContext& Begin(const std::wstring& ID = L"", bool Async = false);


    };
}