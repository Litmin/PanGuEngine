#pragma once
#include "DescriptorHeapAllocation.h"

namespace RHI 
{
    /**
    * Direct3D 12 Descriptor HeapµÄ·â×°
    */
    class DescriptorHeap
    {
    public:
        virtual DescriptorHeapAllocation Allocate(UINT32 count)  = 0;
        virtual void Free(DescriptorHeapAllocation&& allocation) = 0;
        virtual UINT32 GetDescriptorSize() const                 = 0;
    };


    // CPU descriptor heap is intended to provide storage for resource view descriptor handles.
    // It contains a pool of DescriptorHeapAllocationManager object instances, where every instance manages
    // its own CPU-only D3D12 descriptor heap:
    //
    //           m_HeapPool[0]                m_HeapPool[1]                 m_HeapPool[2]
    //   |  X  X  X  X  X  X  X  X |, |  X  X  X  O  O  X  X  O  |, |  X  O  O  O  O  O  O  O  |
    //
    //    X - used descriptor                m_AvailableHeaps = {1,2}
    //    O - available descriptor
    //
    // Allocation routine goes through the list of managers that have available descriptors and tries to process
    // the request using every manager. If there are no available managers or no manager was able to handle the request,
    // the function creates a new descriptor heap manager and lets it handle the request
    //
    // Render device contains four CPUDescriptorHeap object instances (one for each D3D12 heap type). The heaps are accessed
    // when a texture or a buffer view is created.
    //
    class CPUDescriptorHeap final : public DescriptorHeap
    {
    public:

    private:

    };

    class GPUDescriptorHeap final : public DescriptorHeap
    {

    };

}