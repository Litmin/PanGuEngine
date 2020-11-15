#include "pch.h"
#include "DescriptorHeap.h"

using namespace Microsoft::WRL;

namespace RHI
{
	DescriptorHeapAllocationManager::DescriptorHeapAllocationManager(RenderDevice& renderDevice, 
																	 IDescriptorAllocator& parentAllocator, 
																	 size_t thisManagerId, 
																	 const D3D12_DESCRIPTOR_HEAP_DESC& heapDesc) :
		m_ParentAllocator				{parentAllocator},
		m_RenderDevice					{renderDevice},
		m_ThisManagerId					{thisManagerId},
		m_HeapDesc						{heapDesc},
		m_DescriptorSize				{m_RenderDevice.GetD3D12Device()->GetDescriptorHandleIncrementSize(heapDesc.Type)},
		m_NumDescriptorsInAllocation	{heapDesc.NumDescriptors},
		m_FreeBlockManager				{heapDesc.NumDescriptors}
	{
		auto d3d12Device = renderDevice.GetD3D12Device();

		m_FirstCPUHandle.ptr = 0;
		m_FirstGPUHandle.ptr = 0;

		d3d12Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_DescriptorHeap));
		m_FirstCPUHandle = m_DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		if (m_HeapDesc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
			m_FirstGPUHandle = m_DescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	}

	DescriptorHeapAllocationManager::DescriptorHeapAllocationManager(RenderDevice& renderDevice, 
																	 IDescriptorAllocator& parentAllocator, 
																	 size_t thisManagerId, 
																	 ID3D12DescriptorHeap* descriptorHeap, 
																	 UINT32 firstDescriptor, 
																	 UINT32 numDescriptors) :
		m_ParentAllocator	{parentAllocator},
		m_RenderDevice	{renderDevice},
		m_ThisManagerId	{thisManagerId},
		m_HeapDesc	{descriptorHeap->GetDesc()},
		m_DescriptorSize	{renderDevice.GetD3D12Device()->GetDescriptorHandleIncrementSize(m_HeapDesc.Type)},
		m_NumDescriptorsInAllocation	{numDescriptors},
		m_FreeBlockManager	{numDescriptors},
		m_DescriptorHeap	{descriptorHeap}
	{
		m_FirstCPUHandle = m_DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		m_FirstCPUHandle.ptr += m_DescriptorSize * firstDescriptor;

		if (m_HeapDesc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
		{
			m_FirstGPUHandle = m_DescriptorHeap->GetGPUDescriptorHandleForHeapStart();
			m_FirstGPUHandle.ptr += m_DescriptorSize * firstDescriptor;
		}
	}

	DescriptorHeapAllocationManager::~DescriptorHeapAllocationManager()
	{
		// 这样在断言时输出自定义信息
		assert(m_FreeBlockManager.GetFreeSize() == m_NumDescriptorsInAllocation && "Not all descriptors were released");
	}

	DescriptorHeapAllocation DescriptorHeapAllocationManager::Allocate(UINT32 count)
	{
		// Descriptor不需要对齐，传入1
		auto allocation = m_FreeBlockManager.Allocate(count, 1);
		if (!allocation.IsValid())
			return DescriptorHeapAllocation();

		assert(allocation.size == count);

		// 用allocation的Offset计算这次分配的CPU、GPU Descriptor Handle
		auto CPUHandle = m_FirstCPUHandle;
		CPUHandle.ptr += allocation.unalignedOffset * m_DescriptorSize;

		auto GPUHandle = m_FirstGPUHandle;
		GPUHandle.ptr += allocation.unalignedOffset * m_DescriptorSize;

		// 分配过的Descriptor最大的数量
		m_MaxAllocatedNum = std::max(m_MaxAllocatedNum, m_FreeBlockManager.GetUsedSize());

		return DescriptorHeapAllocation(m_ParentAllocator, m_DescriptorHeap.Get(), CPUHandle, GPUHandle, count, static_cast<UINT16>(m_ThisManagerId));
	}

	void DescriptorHeapAllocationManager::FreeAllocation(DescriptorHeapAllocation&& allocation)
	{
		assert((allocation.GetAllocationManagerId() == m_ThisManagerId) && "Invalid descriptor heap manager Id");

		if (allocation.IsNull())
			return;

		auto descriptorOffset = (allocation.GetCpuHandle().ptr - m_FirstCPUHandle.ptr) / m_DescriptorSize;
		m_FreeBlockManager.Free(descriptorOffset, allocation.GetNumHandles());

		allocation.Reset();
	}
	CPUDescriptorHeap::CPUDescriptorHeap(RenderDevice& renderDevice, 
										 UINT32 numDescriptorsInHeap, 
										 D3D12_DESCRIPTOR_HEAP_TYPE type, 
										 D3D12_DESCRIPTOR_HEAP_FLAGS flags) :
		m_RenderDevice		{renderDevice},
		m_HeapDesc
		{
			type,
			numDescriptorsInHeap,
			flags,
			1	// NodeMask
		},
		m_DescriptorSize	{renderDevice.GetD3D12Device()->GetDescriptorHandleIncrementSize(type)}
	{
		// 创建一个AllocationManager
		m_HeapPool.emplace_back(renderDevice, *this, 0, m_HeapDesc);
		m_AvailableHeaps.insert(0);
	}

	CPUDescriptorHeap::~CPUDescriptorHeap()
	{
		assert((m_CurrentSize == 0) && "Not all allocations released");
		assert((m_AvailableHeaps.size() == m_HeapPool.size()) && "Not all descriptor heap pools are released");
	}

	DescriptorHeapAllocation CPUDescriptorHeap::Allocate(uint32_t count)
	{
		DescriptorHeapAllocation allocation;

		// 遍历所有有空闲Descriptor的DescriptorHeapManager
		auto availableHeapIt = m_AvailableHeaps.begin();
		while (availableHeapIt != m_AvailableHeaps.end())
		{
			// 缓存下一个迭代器，可能导致迭代器失效
			auto nextIt = availableHeapIt;
			++nextIt;
			// 尝试使用当前Manager分配Descriptor
			allocation = m_HeapPool[*availableHeapIt].Allocate(count);
			// 如果当前Manager没有空闲的Descriptor，就移除它
			if (m_HeapPool[*availableHeapIt].GetNumAvailableDescriptors() == 0)
				m_AvailableHeaps.erase(*availableHeapIt);

			// 如果分配成功，就停止循环
			if (!allocation.IsNull())
				break;

			availableHeapIt = nextIt;
		}

		// 如果没有可用的DescriptorHeapManager，或者没有DescriptorHeapManager能处理这次分配，就创建一个新的Manager
		if (allocation.IsNull())
		{
			// 当前Descriptor Heap的大小小于要分配的Descriptor的数量，新创建的Descriptor Heap需要满足这次分配的数量
			if (count > m_HeapDesc.NumDescriptors)
			{
				LOG("Increasing the number of descriptors in the heap");
			}

			m_HeapDesc.NumDescriptors = std::max(m_HeapDesc.NumDescriptors, static_cast<UINT>(count));
			// 创建一个新的DescriptorHeapAllocationManager，这会创建一个新的DX12 Descriptor Heap，使用m_HeapPool的索引作为ManagerID
			m_HeapPool.emplace_back(m_RenderDevice, *this, m_HeapPool.size(), m_HeapDesc);
			m_AvailableHeaps.insert(m_HeapPool.size() - 1);

			allocation = m_HeapPool[m_HeapPool.size() - 1].Allocate(count);
		}

		m_CurrentSize += static_cast<UINT32>(allocation.GetNumHandles());
		m_MaxSize = std::max(m_MaxSize, m_CurrentSize);

		return allocation;
	}

	void CPUDescriptorHeap::Free(DescriptorHeapAllocation&& allocation)
	{
		struct StaleAllocation
		{
			DescriptorHeapAllocation Allocation;
			CPUDescriptorHeap* Heap;

			StaleAllocation(DescriptorHeapAllocation&& _Allocation, CPUDescriptorHeap& _Heap)noexcept :
				Allocation{ std::move(_Allocation) },
				Heap{ &_Heap }
			{
			}

			StaleAllocation(const StaleAllocation&) = delete;
			StaleAllocation& operator= (const StaleAllocation&) = delete;
			StaleAllocation& operator= (StaleAllocation&&) = delete;

			StaleAllocation(StaleAllocation&& rhs)noexcept :
				Allocation{ std::move(rhs.Allocation) },
				Heap{ rhs.Heap }
			{
				rhs.Heap = nullptr;
			}

			~StaleAllocation()
			{
				// 析构函数中真正释放
				if (Heap != nullptr)
					Heap->FreeAllocation(std::move(Allocation));
			}
		};

		m_RenderDevice.SafeReleaseDeviceObject(StaleAllocation{ std::move(allocation), *this });
	}

	void CPUDescriptorHeap::FreeAllocation(DescriptorHeapAllocation&& allocation)
	{
		auto managerID = allocation.GetAllocationManagerId();
		m_CurrentSize -= static_cast<UINT32>(allocation.GetNumHandles());
		m_HeapPool[managerID].FreeAllocation(std::move(allocation));
		// 插入失败也没关系
		m_AvailableHeaps.insert(managerID);
	}

	GPUDescriptorHeap::GPUDescriptorHeap(RenderDevice& renderDevice, 
										 UINT32 numDescriptorsInHeap, 
										 UINT32 numDynamicDescriptors, 
										 D3D12_DESCRIPTOR_HEAP_TYPE type, 
										 D3D12_DESCRIPTOR_HEAP_FLAGS flags) :
		m_RenderDevice	{renderDevice},
		m_HeapDesc
		{
			type,
			numDescriptorsInHeap + numDynamicDescriptors,
			flags,
			1 // NodeMask
		},
		// ???
		m_DescriptorHeap
		{
			[&]
			{
			  ComPtr<ID3D12DescriptorHeap> descriptorHeap;
			  renderDevice.GetD3D12Device()->CreateDescriptorHeap(&m_HeapDesc, IID_PPV_ARGS(&descriptorHeap));
			  return descriptorHeap;
			}()
		},
		m_DescriptorSize	{renderDevice.GetD3D12Device()->GetDescriptorHandleIncrementSize(type)},
		m_HeapAllocationManager	{renderDevice, *this, 0, m_DescriptorHeap.Get(), 0, numDescriptorsInHeap},
		m_DynamicAllocationsManager	{renderDevice, *this, 1, m_DescriptorHeap.Get(), numDescriptorsInHeap, numDynamicDescriptors}

	{
	}

	GPUDescriptorHeap::~GPUDescriptorHeap()
	{

	}

	void GPUDescriptorHeap::Free(DescriptorHeapAllocation&& allocation)
	{
		struct StaleAllocation
		{
			DescriptorHeapAllocation Allocation;
			GPUDescriptorHeap* Heap;

			// clang-format off
			StaleAllocation(DescriptorHeapAllocation&& _Allocation, GPUDescriptorHeap& _Heap)noexcept :
				Allocation{ std::move(_Allocation) },
				Heap{ &_Heap }
			{
			}

			StaleAllocation(const StaleAllocation&) = delete;
			StaleAllocation& operator= (const StaleAllocation&) = delete;
			StaleAllocation& operator= (StaleAllocation&&) = delete;

			StaleAllocation(StaleAllocation&& rhs)noexcept :
				Allocation{ std::move(rhs.Allocation) },
				Heap{ rhs.Heap }
			{
				rhs.Heap = nullptr;
			}
			// clang-format on

			~StaleAllocation()
			{
				// 析构函数真正释放Descriptor
				if (Heap != nullptr)
				{
					auto MgrId = Allocation.GetAllocationManagerId();
					assert((MgrId == 0 || MgrId == 1) && "Unexpected allocation manager ID");

					if (MgrId == 0)
					{
						Heap->m_HeapAllocationManager.FreeAllocation(std::move(Allocation));
					}
					else
					{
						Heap->m_DynamicAllocationsManager.FreeAllocation(std::move(Allocation));
					}
				}
			}
		};

		m_RenderDevice.SafeReleaseDeviceObject(StaleAllocation{ std::move(allocation), *this });
	}

	// 构造函数中不分配Chunk
	DynamicSuballocationsManager::DynamicSuballocationsManager(GPUDescriptorHeap& parentGPUHeap, 
															   UINT32 dynamicChunkSize, 
															   std::string managerName) :
		m_ParentGPUHeap	{parentGPUHeap},
		m_DynamicChunkSize	{dynamicChunkSize},
		m_ManagerName	{managerName}
	{
	}

	DynamicSuballocationsManager::~DynamicSuballocationsManager()
	{
		assert(m_Chunks.empty() && m_CurrDescriptorCount == 0 && m_CurrChunkSize == 0 && "All dynamic suballocations must be released!");
	}

	// 释放所有分配的Chunk，这些Chunk会添加到释放队列中，最终会被ParentGPUHeap回收
	void DynamicSuballocationsManager::ReleaseAllocations()
	{
		for (auto& Allocation : m_Chunks)
		{
			m_ParentGPUHeap.Free(std::move(Allocation));
		}
		m_Chunks.clear();
		m_CurrDescriptorCount = 0;
		m_CurrChunkSize = 0;
	}

	// 可能会浪费Chunk的空间，例如倒数第二个Chunk还有空闲内存，但是不足以存放这次分配，
	// 这会创建一个新的Chunk，之后的分配都是在这个新Chunk中进行，这样就浪费了前一个Chunk的一部分空闲内存
	DescriptorHeapAllocation DynamicSuballocationsManager::Allocate(UINT32 count)
	{
		// 如果没有Chunk或者最后一个Chunk没有足够的空间，就分配一个足够大的Chunk
		if (m_Chunks.empty() ||
			m_CurrentOffsetInChunk + count > m_Chunks.back().GetNumHandles())
		{
			auto newChunkSize = std::max(m_DynamicChunkSize, count);
			auto newChunk = m_ParentGPUHeap.AllocateDynamic(newChunkSize);
			if (newChunk.IsNull())
			{
				LOG_ERROR("GPU Descriptor heap is full.");
				return DescriptorHeapAllocation();
			}

			m_Chunks.emplace_back(std::move(newChunk));
			m_CurrentOffsetInChunk = 0;

			m_CurrChunkSize += newChunkSize;
			// 分配的Chunk大小峰值
			m_PeakSuballocationsTotalSize = std::max(m_PeakSuballocationsTotalSize, m_CurrChunkSize);
		}

		// 在最后一个Chunk中进行子分配
		auto& currentSuballocation = m_Chunks.back();

		auto managerID = currentSuballocation.GetAllocationManagerId();

		DescriptorHeapAllocation allocation(*this, 
											currentSuballocation.GetDescriptorHeap(), 
											currentSuballocation.GetCpuHandle(m_CurrentOffsetInChunk),
											currentSuballocation.GetGpuHandle(m_CurrentOffsetInChunk), 
											count, 
											static_cast<UINT16>(managerID));
		m_CurrentOffsetInChunk += count;
		m_CurrDescriptorCount += count;
		m_PeakDescriptorCount = std::max(m_PeakDescriptorCount, m_CurrDescriptorCount);

		return allocation;
	}
}

