#pragma once
#include "DescriptorHeap.h"
#include "PipelineState.h"
#include "GpuBuffer.h"
#include "GpuTexture.h"
#include "DescriptorHeap.h"

namespace RHI 
{
	class CommandContext;
	class GraphicContext;
	class ComputeContext;

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
			assert(m_Type != D3D12_COMMAND_LIST_TYPE_COMPUTE, "Cannot convert async compute context to graphics");
			return reinterpret_cast<GraphicsContext&>(*this);
		}

		ComputeContext& GetComputeContext() 
		{
			return reinterpret_cast<ComputeContext&>(*this);
		}


		// TODO: Implement ��ʼ����Դ
		static void InitializeBuffer(GpuBuffer& Dest, const void* Data, size_t NumBytes, size_t DestOffset = 0);
		static void InitializeBuffer(GpuBuffer& Dest, const UploadBuffer& Src, size_t SrcOffset, size_t NumBytes = -1, size_t DestOffset = 0);
		static void InitializeTexture(GpuResource& Dest, UINT NumSubresources, D3D12_SUBRESOURCE_DATA SubData[]);

		// Dynamic Descriptor��GPUDescriptorHeap�Ϸ���,��Finish���ͷ�
		DescriptorHeapAllocation AllocateDynamicGPUVisibleDescriptor(UINT Count = 1);

		// Resource Barrier


		// ��Ⱦ״̬����Դ��
		void SetPipelineState();


	private:
		void SetID(const std::wstring& ID) { m_ID = ID; }

		D3D12_COMMAND_LIST_TYPE m_Type;
		// CommandList��CommandContext���У�CommandAllocator�ɶ���ع���
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;
		ID3D12CommandAllocator* m_CurrentAllocator;

		// ResourceBarrier����16��ʱFlush
		D3D12_RESOURCE_BARRIER m_ResourceBarrierBuffer[16];
		UINT m_NumBarriersToFlush;

		// Dynamic Descriptor
		DynamicSuballocationsManager* m_DynamicGPUDescriptorAllocator;

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