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


	// Compute Command Listֻ֧���⼸��״̬�Ĺ���
	#define VALID_COMPUTE_QUEUE_RESOURCE_STATES \
	    ( D3D12_RESOURCE_STATE_UNORDERED_ACCESS \
	    | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE \
		| D3D12_RESOURCE_STATE_COPY_DEST \
		| D3D12_RESOURCE_STATE_COPY_SOURCE )


	// TODO:�ù����ƶ���RenderDevice��
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
    * CommandList��CommandListAllocator�ļ�����
	* ����Static����Begin������һ��CommandContext��Ȼ����Կ�ʼ¼���������End����������CommandQueue
	* Beginʱ��������һ���µ�Allocator��Endʱ�������Allocator
	* ÿ���߳�ʹ���Լ���CommandContext
    */
    class CommandContext : public NonCopyable
    {
		friend ContextManager;

		// CommandContext��ContextManager����,���԰ѹ��캯���ķ���Ȩ����Ϊprivate
	private:
		CommandContext(D3D12_COMMAND_LIST_TYPE Type);
		// ����CommandContextʱ���ã��ú����ᴴ��һ��CommandList��������һ��Allocator
		void Initialize();
		// ����CommandContextʱ���ã�������Ⱦ״̬���ú���������һ��Allocator��������CommandList::Reset
		void Reset();

	public:
		// ��ʼ��¼����
		 static CommandContext& Begin(const std::wstring ID = L"");
		// Flush existing commands to the GPU but keep the context alive
		uint64_t Flush(bool WaitForCompletion = false);
		// ��ɼ�¼����
		uint64_t Finish(bool WaitForCompletion = false);


		// Graphic Context����ת����Compute Context������Compute Context����ת����Graphic Context
		GraphicsContext& GetGraphicsContext() 
		{
			assert(m_Type != D3D12_COMMAND_LIST_TYPE_COMPUTE && "Cannot convert async compute context to graphics");
			return reinterpret_cast<GraphicsContext&>(*this);
		}

		ComputeContext& GetComputeContext() 
		{
			return reinterpret_cast<ComputeContext&>(*this);
		}


		// TODO: Implement ��ʼ����Դ
		static void InitializeBuffer(GpuBuffer& Dest, const void* Data, size_t NumBytes, size_t DestOffset = 0);
		static void InitializeBuffer(GpuBuffer& Dest, const GpuUploadBuffer& Src, size_t SrcOffset, size_t NumBytes = -1, size_t DestOffset = 0);
		static void InitializeTexture(GpuResource& Dest, UINT NumSubresources, D3D12_SUBRESOURCE_DATA SubData[]);


		// Dynamic Descriptor��GPUDescriptorHeap�Ϸ���,��Finish���ͷ�
		DescriptorHeapAllocation AllocateDynamicGPUVisibleDescriptor(UINT Count = 1);

		// ΪDynamic Resource�����ڴ�
		D3D12DynamicAllocation AllocateDynamicSpace(size_t NumBytes, size_t Alignment);

		/* Resource Barrier TODO: UAVBarrier
		* GpuResource��������Ա��m_UsageState��m_TransitioningState��
		* m_UsageState:��ʾ��Դ��ǰ��״̬������TransitionResource����ʹ��m_UsageState�����Դ�ĵ�ǰ״̬�͹���֮���״̬���������Ȳ��ύResource Barrier
		* m_TransitioningState:��ʾ��Դ���ڹ��ȵ�״̬������Split Barrier�������Դ�Ѿ�������Begin Barrier
		* BeginResourceTransitionʹ��Split Barrier���Ż�����
		* https://docs.microsoft.com/en-us/windows/win32/direct3d12/using-resource-barriers-to-synchronize-resource-states-in-direct3d-12
		* �ĵ�����˵����Applications should batch multiple transitions into one API call wherever possible.
		* TODO: �������ﾡ�����浽16���Ժ���һ��ִ��(FlushResourceBarriers),�������ԭ��
		*/
		void TransitionResource(GpuResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate = false);
		void BeginResourceTransition(GpuResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate = false);
		inline void FlushResourceBarriers(void);


		/*��Ⱦ״̬����Դ��
		* �л�PSOʱ�����Զ��ύStatic SRB��Static SRBֻ��Static Shader Variable
		* �л�SRBʱ�����Զ��ύMutable SRB����Դ
		* Draw Callǰ�����Զ��ύ��ǰSRB�е�Dynamic Shader Variable��Dynamic Buffer���ύǰ�����Ƿ���Դ�Ƿ��и���
		*/
		void SetPipelineState(PipelineState* PSO);
		void SetShaderResourceBinding(ShaderResourceBinding* SRB);
		void CommitDynamic();	// TODO: Make private

		void SetDescriptorHeap(ID3D12DescriptorHeap* cbvsrvuavHeap, ID3D12DescriptorHeap* samplerHeap);

	protected:
		void SetID(const std::wstring& ID) { m_ID = ID; }

		D3D12_COMMAND_LIST_TYPE m_Type;
		// CommandList��CommandContext���У�CommandAllocator�ɶ���ع���
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;
		ID3D12CommandAllocator* m_CurrentAllocator;

		// ResourceBarrier����16��ʱFlush
		D3D12_RESOURCE_BARRIER m_ResourceBarrierBuffer[16];
		UINT m_NumBarriersToFlush;

		// Dynamic Descriptor
		DynamicSuballocationsManager m_DynamicGPUDescriptorAllocator;

		// Dynamic Resource
		DynamicResourceHeap m_DynamicResourceHeap;

		// ��Դ��
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

		// Vertex Buffer��Index Buffer
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