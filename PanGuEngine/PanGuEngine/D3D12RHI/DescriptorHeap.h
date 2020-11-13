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
    * ��D3D12 Descriptor Heap�н����ӷ���
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
        // ����һ���µ�D3D12 Descriptor Heap
        DescriptorHeapAllocationManager(IMemoryAllocator&                 allocator,
                                        RenderDevice&                     renderDevice,
                                        IDescriptorAllocator&             parentAllocator,
                                        size_t                            thisManagerId,
                                        const D3D12_DESCRIPTOR_HEAP_DESC& heapDesc);

        // ʹ�����е�D3D12 Descriptor Heap�е��ӷ�Χ
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
    * CPU Descriptor�洢��Դ��Descriptor Handle��������һ��DescriptorHeapAllocationManager�Ķ���أ�ÿ���������CPU-only Descriptor Heap��һ���֣�
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

    private:

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

    };

}