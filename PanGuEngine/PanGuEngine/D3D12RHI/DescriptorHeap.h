#pragma once
#include "RenderDevice.h"
#include "VariableSizeAllocationsManager.h"

namespace RHI 
{
    class DescriptorHeapAllocation;


    class IDescriptorAllocator
    {
    public:
        virtual DescriptorHeapAllocation Allocate(UINT32 count)  = 0;
        virtual void Free(DescriptorHeapAllocation&& allocation) = 0;
        virtual UINT32 GetDescriptorSize() const                 = 0;
    };

    /**
    * ��ʾһ��Descriptor Heap�ķ��䣬Ҳ����Descriptor Heap�е�һ�������ķ�Χ
    */
    class DescriptorHeapAllocation
    {
    public:
        // ����һ���յ�Allocation
        DescriptorHeapAllocation() noexcept :
            m_NumHandles        {1},
            m_pDescriptorHeap   { nullptr },
            m_DescriptorSize    { 0 }
        {
            m_FirstCpuHandle.ptr = 0;
            m_FirstGpuHandle.ptr = 0;
        }

        DescriptorHeapAllocation(IDescriptorAllocator&       allocator,
                                 ID3D12DescriptorHeap*       descriptorHeap,
                                 D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
                                 D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle,
                                 UINT32                      numHandles,
                                 UINT16                      allocationManagerID) noexcept :
            m_FirstCpuHandle        {cpuHandle},
            m_FirstGpuHandle        {gpuHandle},
            m_pAllocator            {&allocator},
            m_NumHandles            {numHandles},
            m_pDescriptorHeap       { descriptorHeap },
            m_AllocationManagerId   { allocationManagerID }
        {
            auto DescriptorSize = m_pAllocator->GetDescriptorSize();
            m_DescriptorSize = static_cast<UINT16>(DescriptorSize);
        }

        // �ƶ����캯������������)
        DescriptorHeapAllocation(DescriptorHeapAllocation&& allocation) noexcept :
            m_FirstCpuHandle      {std::move(allocation.m_FirstCpuHandle)     },
            m_FirstGpuHandle      {std::move(allocation.m_FirstGpuHandle)     },
            m_NumHandles          {std::move(allocation.m_NumHandles)         },
            m_pAllocator          {std::move(allocation.m_pAllocator)         },
            m_AllocationManagerId {std::move(allocation.m_AllocationManagerId)},
            m_pDescriptorHeap     {std::move(allocation.m_pDescriptorHeap)    },
            m_DescriptorSize      {std::move(allocation.m_DescriptorSize)     }
        {
            allocation.Reset();
        }

        // �ƶ���ֵ��������������ƣ�
        DescriptorHeapAllocation& operator=(DescriptorHeapAllocation&& allocation) noexcept
        {
            m_FirstCpuHandle = std::move(allocation.m_FirstCpuHandle);
            m_FirstGpuHandle = std::move(allocation.m_FirstGpuHandle);
            m_NumHandles = std::move(allocation.m_NumHandles);
            m_pAllocator = std::move(allocation.m_pAllocator);
            m_AllocationManagerId = std::move(allocation.m_AllocationManagerId);
            m_pDescriptorHeap = std::move(allocation.m_pDescriptorHeap);
            m_DescriptorSize = std::move(allocation.m_DescriptorSize);

            allocation.Reset();

            return *this;
        }

        // Destructor automatically releases this allocation through the allocator
        ~DescriptorHeapAllocation()
        {
            // �ͷ���η���
            if (!IsNull() && m_pAllocator)
                m_pAllocator->Free(std::move(*this));
        }

        // ��ֹ����
        DescriptorHeapAllocation(const DescriptorHeapAllocation&) = delete;
        DescriptorHeapAllocation& operator=(const DescriptorHeapAllocation&) = delete;

        void Reset()
        {
            m_FirstCpuHandle.ptr = 0;
            m_FirstGpuHandle.ptr = 0;
            m_pAllocator = nullptr;
            m_pDescriptorHeap = nullptr;
            m_NumHandles = 0;
            m_AllocationManagerId = static_cast<UINT16>(-1);
            m_DescriptorSize = 0;
        }

        // ����ָ��Offset��CPU Descriptor Handle
        D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(UINT32 Offset = 0) const
        {
            D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle = m_FirstCpuHandle;
            CPUHandle.ptr += m_DescriptorSize * Offset;

            return CPUHandle;
        }

        // ����ָ��Offset��GPU Descriptor Handle
        D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(UINT32 Offset = 0) const
        {
            D3D12_GPU_DESCRIPTOR_HANDLE GPUHandle = m_FirstGpuHandle;
            GPUHandle.ptr += m_DescriptorSize * Offset;

            return GPUHandle;
        }

        ID3D12DescriptorHeap* GetDescriptorHeap() { return m_pDescriptorHeap; }

        size_t GetNumHandles()          const { return m_NumHandles; }
        bool   IsNull()                 const { return m_FirstCpuHandle.ptr == 0; }
        bool   IsShaderVisible()        const { return m_FirstGpuHandle.ptr != 0; }
        size_t GetAllocationManagerId() const { return m_AllocationManagerId; }
        UINT  GetDescriptorSize()       const { return m_DescriptorSize; }

    private:
        // First CPU descriptor handle in this allocation
        D3D12_CPU_DESCRIPTOR_HANDLE m_FirstCpuHandle = { 0 };

        // First GPU descriptor handle in this allocation
        D3D12_GPU_DESCRIPTOR_HANDLE m_FirstGpuHandle = { 0 };

        // Pointer to the descriptor heap allocator that created this allocation
        IDescriptorAllocator* m_pAllocator = nullptr;

        // Pointer to the D3D12 descriptor heap that contains descriptors in this allocation
        ID3D12DescriptorHeap* m_pDescriptorHeap = nullptr;

        // Number of descriptors in the allocation
        UINT32 m_NumHandles = 0;

        // Allocation manager ID. One allocator may support several
        // allocation managers. This field is required to identify
        // the manager within the allocator that was used to create
        // this allocation
        UINT16 m_AllocationManagerId = static_cast<UINT16>(-1);

        // Descriptor size
        UINT16 m_DescriptorSize = 0;
    };

    /**
    * ����һ��������DX12 Descriptor Heap������һ��Descriptor Heap��һ����
    * ��CPU-only��Descriptor Heap�ж��ǹ���һ��������Descriptor Heap
    * ��GPU-visible��Descriptor Heap�У���Ϊֻ��һ��DX12 Descriptor Heap������Managerֻ�ǹ������е�һ����
    * ʹ��VariableSizeAllocationsManager������еĿ����ڴ�
    *
    * |  X  X  X  X  O  O  O  X  X  O  O  X  O  O  O  O  |  D3D12 descriptor heap
    *
    *  X - used descriptor
    *  O - available descriptor
    */
    class DescriptorHeapAllocationManager
    {
    public:
        // ����һ���µ�D3D12 Descriptor Heap, CPU Descriptor Heapʹ��
        DescriptorHeapAllocationManager(RenderDevice&                     renderDevice,
                                        IDescriptorAllocator&             parentAllocator,
                                        size_t                            thisManagerId,
                                        const D3D12_DESCRIPTOR_HEAP_DESC& heapDesc);

        // ʹ�����е�D3D12 Descriptor Heap�е��ӷ�Χ, GPU Descriptor Heapʹ��
        DescriptorHeapAllocationManager(RenderDevice&         renderDevice,
                                        IDescriptorAllocator& parentAllocator,
                                        size_t                thisManagerId,
                                        ID3D12DescriptorHeap* descriptorHeap,
                                        UINT32                firstDescriptor,
                                        UINT32                numDescriptors);

        DescriptorHeapAllocationManager(DescriptorHeapAllocationManager&& rhs) noexcept :
            m_ParentAllocator           { rhs.m_ParentAllocator },
            m_RenderDevice              { rhs.m_RenderDevice },
            m_ThisManagerId             { rhs.m_ThisManagerId },
            m_HeapDesc                  { rhs.m_HeapDesc },
            m_DescriptorSize            { rhs.m_DescriptorSize },
            m_NumDescriptorsInAllocation{ rhs.m_NumDescriptorsInAllocation },
            m_FirstCPUHandle            { rhs.m_FirstCPUHandle },
            m_FirstGPUHandle            { rhs.m_FirstGPUHandle },
            m_MaxAllocatedNum          { rhs.m_MaxAllocatedNum },
            m_FreeBlockManager          { std::move(rhs.m_FreeBlockManager) },
            m_DescriptorHeap            { std::move(rhs.m_DescriptorHeap) }
        {
            rhs.m_NumDescriptorsInAllocation = 0; 
            rhs.m_ThisManagerId = static_cast<size_t>(-1);
            rhs.m_FirstCPUHandle.ptr = 0;
            rhs.m_FirstGPUHandle.ptr = 0;
            rhs.m_MaxAllocatedNum = 0;
        }

        DescriptorHeapAllocationManager& operator = (DescriptorHeapAllocationManager&&) = delete;
        DescriptorHeapAllocationManager(const DescriptorHeapAllocationManager&) = delete;
        DescriptorHeapAllocationManager& operator = (const DescriptorHeapAllocationManager&) = delete;

        ~DescriptorHeapAllocationManager();


        DescriptorHeapAllocation Allocate(UINT32 count);
        void                     FreeAllocation(DescriptorHeapAllocation&& allocation);

        size_t GetNumAvailableDescriptors()const { return m_FreeBlockManager.GetFreeSize(); }
        UINT32 GetMaxDescriptors()         const { return m_NumDescriptorsInAllocation; }
        size_t GetMaxAllocatedSize()       const { return m_MaxAllocatedNum; }

    private:
        IDescriptorAllocator& m_ParentAllocator;
        RenderDevice& m_RenderDevice;

        size_t m_ThisManagerId = static_cast<size_t>(-1);

        const D3D12_DESCRIPTOR_HEAP_DESC m_HeapDesc;

        const UINT m_DescriptorSize = 0;

        // ���Է����Descriptor������
        UINT32 m_NumDescriptorsInAllocation = 0;

        VariableSizeAllocationsManager m_FreeBlockManager;

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DescriptorHeap;

        D3D12_CPU_DESCRIPTOR_HANDLE m_FirstCPUHandle = { 0 };
        D3D12_GPU_DESCRIPTOR_HANDLE m_FirstGPUHandle = { 0 };

        // �Ѿ��������Descriptor���������
        size_t m_MaxAllocatedNum = 0;
    };


    /**
    * CPUDescriptorHeap�洢��Դ��Descriptor Handle��
    * ������һ��DescriptorHeapAllocationManager�Ķ���أ�ÿ��Manager���ᴴ��һ��DX12 Descriptor Heap��ÿ���������CPU-only Descriptor Heap��һ���֣�
    *         m_HeapPool[0]                m_HeapPool[1]                 m_HeapPool[2]
    * |  X  X  X  X  X  X  X  X |, |  X  X  X  O  O  X  X  O  |, |  X  O  O  O  O  O  O  O  |
    *
    *  X - used descriptor                m_AvailableHeaps = {1,2}
    *  O - available descriptor
    * ÿ�η��䶼�����Manager�б�����ʹ��Manager�����䡣���û�п��õ�Manager����û��Manager���Դ����������
    * �����ᴴ��һ���µ�DescriptorHeapAllocationManager�������������
    *
    * Render Device�����ĸ�CPUDescriptorHeap���󣬶�ӦD3D12������Descriptor Heap��SRV_CBV_UAV,Sampler,RTV,DSV����
    */
    class CPUDescriptorHeap final : public IDescriptorAllocator
    {
    public:
        CPUDescriptorHeap(RenderDevice& renderDevice,
                          UINT32 numDescriptorsInHeap,
                          D3D12_DESCRIPTOR_HEAP_TYPE type,
                          D3D12_DESCRIPTOR_HEAP_FLAGS flags);

        CPUDescriptorHeap(const CPUDescriptorHeap&) = delete;
        CPUDescriptorHeap(CPUDescriptorHeap&&) = delete;
        CPUDescriptorHeap& operator = (const CPUDescriptorHeap&) = delete;
        CPUDescriptorHeap& operator = (CPUDescriptorHeap&&) = delete;

        ~CPUDescriptorHeap();

        virtual DescriptorHeapAllocation Allocate(uint32_t count) override final;
        virtual void                     Free(DescriptorHeapAllocation&& allocation) override final;
        virtual UINT32                   GetDescriptorSize() const override final { return m_DescriptorSize; }

    private:
        void FreeAllocation(DescriptorHeapAllocation&& allocation);

        RenderDevice& m_RenderDevice;

        std::vector<DescriptorHeapAllocationManager> m_HeapPool;
        std::unordered_set<size_t, std::hash<size_t>, std::equal_to<size_t>> m_AvailableHeaps;

        D3D12_DESCRIPTOR_HEAP_DESC m_HeapDesc;
        const UINT                 m_DescriptorSize = 0;

        UINT32 m_MaxSize = 0;
        // ��ǰ�����Descriptor����
        UINT32 m_CurrentSize = 0;
    };

    /**
    * GPUDescriptorHeap�洢Shader�ɼ���Descriptor
    * ������һ��D3D12 Descriptor Heap�����ֳ���������
    * ��һ���ִ洢Static��Mutable Resource Descriptor Handle
    * �ڶ����ִ洢Dynamic Resource Descriptor Handle
    * Dynamic Resource��ÿ���߳��ȷ���һ��Chunk��Ȼ����Chunk�н����ӷ��䣬DynamicSuballocationsManager��ʵ���������
    * 
    *   static and mutable handles      ||                 dynamic space
    *                                   ||    chunk 0     chunk 1     chunk 2     unused
    *| X O O X X O X O O O O X X X X O  ||  | X X X O | | X X O O | | O O O O |  O O O O  ||
    *                                             |         |
    *                                   suballocation       suballocation
    *                                  within chunk 0       within chunk 1
    * Render Device��������GPUDescriptorHeap����SRV_CBV_UAV��Sampler����Heap����ΪShader Resource Binding�������Shader�ɼ���Descriptor��
    * Device Context��������Dynamic Resource Descriptor
    *
    *  _______________________________________________________________________________________________________________________________
    * | Render Device                                                                                                                 |
    * |                                                                                                                               |
    * | m_CPUDescriptorHeaps[CBV_SRV_UAV] |  X  X  X  X  X  X  X  X  |, |  X  X  X  X  X  X  X  X  |, |  X  O  O  X  O  O  O  O  |    |
    * | m_CPUDescriptorHeaps[SAMPLER]     |  X  X  X  X  O  O  O  X  |, |  X  O  O  X  O  O  O  O  |                                  |
    * | m_CPUDescriptorHeaps[RTV]         |  X  X  X  O  O  O  O  O  |, |  O  O  O  O  O  O  O  O  |                                  |
    * | m_CPUDescriptorHeaps[DSV]         |  X  X  X  O  X  O  X  O  |                                                                |
    * |                                                                               ctx1        ctx2                                |
    * | m_GPUDescriptorHeaps[CBV_SRV_UAV]  | X O O X X O X O O O O X X X X O  ||  | X X X O | | X X O O | | O O O O |  O O O O  ||    |
    * | m_GPUDescriptorHeaps[SAMPLER]      | X X O O X O X X X O O X O O O O  ||  | X X O O | | X O O O | | O O O O |  O O O O  ||    |
    * |                                                                                                                               |
    * |_______________________________________________________________________________________________________________________________|
    *
    *  ________________________________________________               ________________________________________________
    * |Device Context 1                                |             |Device Context 2                                |
    * |                                                |             |                                                |
    * | m_DynamicGPUDescriptorAllocator[CBV_SRV_UAV]   |             | m_DynamicGPUDescriptorAllocator[CBV_SRV_UAV]   |
    * | m_DynamicGPUDescriptorAllocator[SAMPLER]       |             | m_DynamicGPUDescriptorAllocator[SAMPLER]       |
    * |________________________________________________|             |________________________________________________|
    */
    class GPUDescriptorHeap final : public IDescriptorAllocator
    {
    public:
        GPUDescriptorHeap(RenderDevice& renderDevice,
                          UINT32 numDescriptorsInHeap,
                          UINT32 numDynamicDescriptors,
                          D3D12_DESCRIPTOR_HEAP_TYPE type,
                          D3D12_DESCRIPTOR_HEAP_FLAGS flags);

        GPUDescriptorHeap(const GPUDescriptorHeap&) = delete;
        GPUDescriptorHeap(GPUDescriptorHeap&&) = delete;
        GPUDescriptorHeap& operator = (const GPUDescriptorHeap&) = delete;
        GPUDescriptorHeap& operator = (GPUDescriptorHeap&&) = delete;

        ~GPUDescriptorHeap();

        virtual DescriptorHeapAllocation Allocate(UINT32 count) override final
        {
            return m_HeapAllocationManager.Allocate(count);
        }

        virtual void   Free(DescriptorHeapAllocation&& allocation) override final;
        virtual UINT32 GetDescriptorSize() const override final { return m_DescriptorSize; }

        DescriptorHeapAllocation AllocateDynamic(UINT32 count)
        {
            return m_DynamicAllocationsManager.Allocate(count);
        }

        const D3D12_DESCRIPTOR_HEAP_DESC& GetHeapDesc() const { return m_HeapDesc; }
        UINT32                            GetMaxStaticDescriptors() const { return m_HeapAllocationManager.GetMaxDescriptors(); }
        UINT32                            GetMaxDynamicDescriptors() const { return m_DynamicAllocationsManager.GetMaxDescriptors(); }

    private:
        RenderDevice& m_RenderDevice;

        // GPU Descriptor Heapֻ�ᴴ��һ��DX12 Descriptor Heap
        const D3D12_DESCRIPTOR_HEAP_DESC m_HeapDesc;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DescriptorHeap;

        const UINT m_DescriptorSize;

        // static/mutable���ֵ�Manager
        DescriptorHeapAllocationManager m_HeapAllocationManager;

        // dynamic���ֵ�Manager
        DescriptorHeapAllocationManager m_DynamicAllocationsManager;
    };

    /**
    * ���ศ������̬��Descriptor Handle������GPU Descriptor Heap������һ��Chunk��Ȼ����Chunk�н������Ե��ӷ��䣬
    * ��Ϊ����Chunk��Ҫ������������Chunk�н����ӷ��䲻��Ҫ������ÿ���߳̾Ϳ��Բ��з��䶯̬Descriptor.
    * ��ÿһ֡��ĩβ�����еķ��䶼�ᱻ����
    *
    */
    class DynamicSuballocationsManager final : public IDescriptorAllocator
    {
    public:
        DynamicSuballocationsManager(GPUDescriptorHeap& parentGPUHeap,
                                     UINT32 dynamicChunkSize,
                                     std::string managerName);

        DynamicSuballocationsManager(const DynamicSuballocationsManager&) = delete;
        DynamicSuballocationsManager(DynamicSuballocationsManager&&) = delete;
        DynamicSuballocationsManager& operator = (const DynamicSuballocationsManager&) = delete;
        DynamicSuballocationsManager& operator = (DynamicSuballocationsManager&&) = delete;

        ~DynamicSuballocationsManager();

        void ReleaseAllocations();

        virtual DescriptorHeapAllocation Allocate(UINT32 count) override final;
        virtual void                     Free(DescriptorHeapAllocation&& Allocation) override final
        {
            // Dynamic allocation���Ƿֱ��ͷŵģ�����ÿ֡��ĩβͨ��ReleaseAllocations()�ͷ�����Chunk
            Allocation.Reset();
        }

        virtual UINT32 GetDescriptorSize() const override final { return m_ParentGPUHeap.GetDescriptorSize(); }

        size_t GetSuballocationCount() const { return m_Chunks.size(); }

    private:
        GPUDescriptorHeap& m_ParentGPUHeap;
        const std::string m_ManagerName;

        // �����Chunk
        std::vector<DescriptorHeapAllocation> m_Chunks;

        // �����һ��Chunk�е�Offset��Offset֮���ǻ�δ�����Descriptor
        UINT32 m_CurrentOffsetInChunk = 0;
        // ��ʼ����ÿ��Chunk��С
        UINT32 m_DynamicChunkSize = 0;

        // ��ǰ���������Descriptor����
        UINT32 m_CurrDescriptorCount = 0;
        // ��ǰ������Chunk��С
        UINT32 m_CurrChunkSize = 0;
        // Chunk�з����Descriptor������ֵ
        UINT32 m_PeakDescriptorCount = 0;
        // �����Chunk��С��ֵ
        UINT32 m_PeakSuballocationsTotalSize = 0;
    };
}