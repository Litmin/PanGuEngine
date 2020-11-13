#pragma once
#include "RenderDevice.h"

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
            if (!IsNull() && m_pAllocator)
                m_pAllocator->Free(std::move(*this));
        }

        // 禁止复制
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
    * 在D3D12 Descriptor Heap中进行子分配
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
        // 创建一个新的D3D12 Descriptor Heap
        DescriptorHeapAllocationManager(IMemoryAllocator&                 allocator,
                                        RenderDevice&                     renderDevice,
                                        IDescriptorAllocator&             parentAllocator,
                                        size_t                            thisManagerId,
                                        const D3D12_DESCRIPTOR_HEAP_DESC& heapDesc);

        // 使用现有的D3D12 Descriptor Heap中的子范围
        DescriptorHeapAllocationManager(IMemoryAllocator&     allocator,
                                        RenderDevice&         renderDevice,
                                        IDescriptorAllocator& parentAllocator,
                                        size_t                thisManagerId,
                                        ID3D12DescriptorHeap* descriptorHeap,
                                        UINT32                firstDescriptor,
                                        UINT                  numDescriptors);


    private:
        IDescriptorAllocator& m_ParentAllocator;
        RenderDevice& m_RenderDevice;

        size_t m_ThisManagerId = static_cast<size_t>(-1);

        const D3D12_DESCRIPTOR_HEAP_DESC m_HeapDesc;

        const UINT m_DescriptorSize = 0;

        UINT32 m_NumDescriptorsInAllocation = 0;

        VariableSizeAllocationsManager m_FreeBlockManager;

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DescriptorHeap;

        D3D12_CPU_DESCRIPTOR_HANDLE m_FirstCPUHandle = { 0 };
        D3D12_GPU_DESCRIPTOR_HANDLE m_FirstGPUHandle = { 0 };

        size_t m_MaxAllocatedSize = 0;
    };


    /**
    * CPU Descriptor存储资源的Descriptor Handle。它包含一个DescriptorHeapAllocationManager的对象池，每个对象管理CPU-only Descriptor Heap的一部分：
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

    private:

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

    };

}