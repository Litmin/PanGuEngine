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
    * 表示一次Descriptor Heap的分配，也就是Descriptor Heap中的一个连续的范围
    */
    class DescriptorHeapAllocation
    {
    public:
        // 创建一个空的Allocation
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

        // 移动构造函数（不允许复制)
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

        // 移动赋值运算符（不允许复制）
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
            // 释放这次分配
            if (!IsNull() && m_pAllocator)
                m_pAllocator->Free(std::move(*this));
        }

        // 禁止拷贝
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

        // 返回指定Offset的CPU Descriptor Handle
        D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(UINT32 Offset = 0) const
        {
            D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle = m_FirstCpuHandle;
            CPUHandle.ptr += m_DescriptorSize * Offset;

            return CPUHandle;
        }

        // 返回指定Offset的GPU Descriptor Handle
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
    * 管理一个完整的DX12 Descriptor Heap，或者一个Descriptor Heap的一部分
    * 在CPU-only的Descriptor Heap中都是管理一个完整的Descriptor Heap
    * 在GPU-visible的Descriptor Heap中，因为只有一个DX12 Descriptor Heap，所以Manager只是管理其中的一部分
    * 使用VariableSizeAllocationsManager管理堆中的空闲内存
    *
    * |  X  X  X  X  O  O  O  X  X  O  O  X  O  O  O  O  |  D3D12 descriptor heap
    *
    *  X - used descriptor
    *  O - available descriptor
    */
    class DescriptorHeapAllocationManager
    {
    public:
        // 创建一个新的D3D12 Descriptor Heap, CPU Descriptor Heap使用
        DescriptorHeapAllocationManager(RenderDevice&                     renderDevice,
                                        IDescriptorAllocator&             parentAllocator,
                                        size_t                            thisManagerId,
                                        const D3D12_DESCRIPTOR_HEAP_DESC& heapDesc);

        // 使用现有的D3D12 Descriptor Heap中的子范围, GPU Descriptor Heap使用
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

        // 可以分配的Descriptor总数量
        UINT32 m_NumDescriptorsInAllocation = 0;

        VariableSizeAllocationsManager m_FreeBlockManager;

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DescriptorHeap;

        D3D12_CPU_DESCRIPTOR_HANDLE m_FirstCPUHandle = { 0 };
        D3D12_GPU_DESCRIPTOR_HANDLE m_FirstGPUHandle = { 0 };

        // 已经分配过的Descriptor的最大数量
        size_t m_MaxAllocatedNum = 0;
    };


    /**
    * CPUDescriptorHeap存储资源的Descriptor Handle。
    * 它包含一个DescriptorHeapAllocationManager的对象池，每个Manager都会创建一个DX12 Descriptor Heap，每个对象管理CPU-only Descriptor Heap的一部分：
    *         m_HeapPool[0]                m_HeapPool[1]                 m_HeapPool[2]
    * |  X  X  X  X  X  X  X  X |, |  X  X  X  O  O  X  X  O  |, |  X  O  O  O  O  O  O  O  |
    *
    *  X - used descriptor                m_AvailableHeaps = {1,2}
    *  O - available descriptor
    * 每次分配都会遍历Manager列表，尝试使用Manager来分配。如果没有可用的Manager或者没有Manager可以处理这次请求，
    * 函数会创建一个新的DescriptorHeapAllocationManager来处理这次请求。
    *
    * Render Device包含四个CPUDescriptorHeap对象，对应D3D12的四种Descriptor Heap（SRV_CBV_UAV,Sampler,RTV,DSV）。
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
        // 当前分配的Descriptor数量
        UINT32 m_CurrentSize = 0;
    };

    /**
    * GPUDescriptorHeap存储Shader可见的Descriptor
    * 它包含一个D3D12 Descriptor Heap，并分成了两部分
    * 第一部分存储Static和Mutable Resource Descriptor Handle
    * 第二部分存储Dynamic Resource Descriptor Handle
    * Dynamic Resource是每个线程先分配一个Chunk，然后在Chunk中进行子分配，DynamicSuballocationsManager来实现这个过程
    * 
    *   static and mutable handles      ||                 dynamic space
    *                                   ||    chunk 0     chunk 1     chunk 2     unused
    *| X O O X X O X O O O O X X X X O  ||  | X X X O | | X X O O | | O O O O |  O O O O  ||
    *                                             |         |
    *                                   suballocation       suballocation
    *                                  within chunk 0       within chunk 1
    * Render Device包含两个GPUDescriptorHeap对象（SRV_CBV_UAV和Sampler）。Heap用来为Shader Resource Binding对象分配Shader可见的Descriptor，
    * Device Context用来分配Dynamic Resource Descriptor
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

        // GPU Descriptor Heap只会创建一个DX12 Descriptor Heap
        const D3D12_DESCRIPTOR_HEAP_DESC m_HeapDesc;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DescriptorHeap;

        const UINT m_DescriptorSize;

        // static/mutable部分的Manager
        DescriptorHeapAllocationManager m_HeapAllocationManager;

        // dynamic部分的Manager
        DescriptorHeapAllocationManager m_DynamicAllocationsManager;
    };

    /**
    * 该类辅助管理动态的Descriptor Handle。它从GPU Descriptor Heap中请求一个Chunk，然后在Chunk中进行线性的子分配，
    * 因为请求Chunk需要加锁，但是在Chunk中进行子分配不需要，这样每个线程就可以并行分配动态Descriptor.
    * 在每一帧的末尾，所有的分配都会被废弃
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
            // Dynamic allocation不是分别释放的，是在每帧的末尾通过ReleaseAllocations()释放整个Chunk
            Allocation.Reset();
        }

        virtual UINT32 GetDescriptorSize() const override final { return m_ParentGPUHeap.GetDescriptorSize(); }

        size_t GetSuballocationCount() const { return m_Chunks.size(); }

    private:
        GPUDescriptorHeap& m_ParentGPUHeap;
        const std::string m_ManagerName;

        // 分配的Chunk
        std::vector<DescriptorHeapAllocation> m_Chunks;

        // 在最后一个Chunk中的Offset，Offset之后是还未分配的Descriptor
        UINT32 m_CurrentOffsetInChunk = 0;
        // 初始化的每个Chunk大小
        UINT32 m_DynamicChunkSize = 0;

        // 当前分配的所有Descriptor数量
        UINT32 m_CurrDescriptorCount = 0;
        // 当前的所有Chunk大小
        UINT32 m_CurrChunkSize = 0;
        // Chunk中分配的Descriptor数量峰值
        UINT32 m_PeakDescriptorCount = 0;
        // 分配的Chunk大小峰值
        UINT32 m_PeakSuballocationsTotalSize = 0;
    };
}