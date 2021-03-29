#include "pch.h"
#include "DynamicResource.h"
#include "RenderDevice.h"

namespace RHI
{

    // 在Upload堆上分配一个Buffer，并Map
    D3D12DynamicPage::D3D12DynamicPage(UINT64 Size)
    {
        D3D12_HEAP_PROPERTIES HeapProps;
        HeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
        HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        HeapProps.CreationNodeMask = 1;
        HeapProps.VisibleNodeMask = 1;

        D3D12_RESOURCE_DESC ResourceDesc;
        ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        ResourceDesc.Width = Size;
        ResourceDesc.Alignment = 0;
        ResourceDesc.Height = 1;
        ResourceDesc.DepthOrArraySize = 1;
        ResourceDesc.MipLevels = 1;
        ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
        ResourceDesc.SampleDesc.Count = 1;
        ResourceDesc.SampleDesc.Quality = 0;
        ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        ID3D12Device* d3dDevice = RenderDevice::GetSingleton().GetD3D12Device();
        ThrowIfFailed(d3dDevice->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &ResourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_pd3d12Buffer)));

        m_pd3d12Buffer->SetName(L"Dynamic memory page");

        m_GPUVirtualAddress = m_pd3d12Buffer->GetGPUVirtualAddress();

        m_pd3d12Buffer->Map(0, nullptr, &m_CPUVirtualAddress);
    }

    DynamicResourceAllocator::DynamicResourceAllocator(UINT32 NumPagesToReserve, UINT64 PageSize)
    {
        for (UINT32 i = 0; i < NumPagesToReserve; ++i)
        {
            D3D12DynamicPage Page(PageSize);
            auto             Size = Page.GetSize();
            m_AvailablePages.emplace(Size, std::move(Page));
        }
    }

    // 返回的Page的大小可能比SizeInBytes大
    D3D12DynamicPage DynamicResourceAllocator::AllocatePage(UINT64 SizeInBytes)
    {
        // 返回比SizeInBytes大的第一个Page的迭代器
        auto PageIt = m_AvailablePages.lower_bound(SizeInBytes);
        if (PageIt != m_AvailablePages.end())
        {
            assert(PageIt->first >= SizeInBytes);
            D3D12DynamicPage Page(std::move(PageIt->second));
            m_AvailablePages.erase(PageIt);
            return Page;
        }
        else
        {
            return D3D12DynamicPage{ SizeInBytes };
        }
    }

    void DynamicResourceAllocator::ReleasePages(std::vector<D3D12DynamicPage>& Pages)
    {
        struct StalePage
        {
            D3D12DynamicPage           Page;
            DynamicResourceAllocator* Mgr;

            StalePage(D3D12DynamicPage&& _Page, DynamicResourceAllocator& _Mgr) noexcept :
                Page{ std::move(_Page) },
                Mgr{ &_Mgr }
            {
            }

            StalePage(const StalePage&) = delete;
            StalePage& operator= (const StalePage&) = delete;
            StalePage& operator= (StalePage&&) = delete;

            StalePage(StalePage&& rhs)noexcept :
                Page{ std::move(rhs.Page) },
                Mgr{ rhs.Mgr }
            {
                rhs.Mgr = nullptr;
            }

            ~StalePage()
            {
                if (Mgr != nullptr)
                {
                    auto PageSize = Page.GetSize();
                    Mgr->m_AvailablePages.emplace(PageSize, std::move(Page));
                }
            }
        };
        for (auto& Page : Pages)
        {
            RenderDevice::GetSingleton().SafeReleaseDeviceObject(StalePage{ std::move(Page), *this });
        }
    }

    void DynamicResourceAllocator::Destroy()
    {
        m_AvailablePages.clear();
    }

    DynamicResourceAllocator::~DynamicResourceAllocator()
    {
        assert(m_AvailablePages.empty() && "Not all pages are destroyed. Dynamic memory manager must be explicitly destroyed with Destroy() method");
    }


    DynamicResourceHeap::~DynamicResourceHeap()
    {
        assert(m_AllocatedPages.empty() && "Allocated pages have not been released which indicates FinishFrame() has not been called");
    }

    D3D12DynamicAllocation DynamicResourceHeap::Allocate(UINT64 SizeInBytes, UINT64 Alignment)
    {
        assert(Alignment > 0);
        assert(IsPowerOfTwo(Alignment) && "Alignment must be power of 2");

        // 如果是第一次分配，或者当前Page的大小不够这次分配，就重新创建一个Page
        if (m_CurrOffset == InvalidOffset || SizeInBytes + (Align(m_CurrOffset, Alignment) - m_CurrOffset) > m_AvailableSize)
        {
            auto NewPageSize = m_BasePageSize;
            // 计算能满足这次分配的Page大小
            while (NewPageSize < SizeInBytes)
                NewPageSize *= 2;

            auto NewPage = m_GlobalDynamicAllocator.AllocatePage(NewPageSize);
            if (NewPage.IsValid())
            {
                m_CurrOffset = 0;
                m_AvailableSize = NewPage.GetSize();

                m_CurrAllocatedSize += m_AvailableSize;
                m_PeakAllocatedSize = std::max(m_PeakAllocatedSize, m_CurrAllocatedSize);

                m_AllocatedPages.emplace_back(std::move(NewPage));
            }
        }

        if (m_CurrOffset != InvalidOffset && SizeInBytes + (Align(m_CurrOffset, Alignment) - m_CurrOffset) <= m_AvailableSize)
        {
            auto AlignedOffset = Align(m_CurrOffset, Alignment);
            auto AdjustedSize = SizeInBytes + (AlignedOffset - m_CurrOffset);
            assert(AdjustedSize <= m_AvailableSize);
            m_AvailableSize -= AdjustedSize;
            m_CurrOffset += AdjustedSize;

            m_CurrUsedSize += SizeInBytes;
            m_PeakUsedSize = std::max(m_PeakUsedSize, m_CurrUsedSize);

            m_CurrUsedAlignedSize += AdjustedSize;
            m_PeakAlignedSize = std::max(m_PeakAlignedSize, m_CurrUsedAlignedSize);

            // 总是从最后一个Page中进行分配
            auto& CurrPage = m_AllocatedPages.back();
            return D3D12DynamicAllocation
            {
                CurrPage.GetD3D12Buffer(),
                AlignedOffset,
                SizeInBytes,
                CurrPage.GetCPUAddress(AlignedOffset),
                CurrPage.GetGPUAddress(AlignedOffset)
            };
        }
        else
            return D3D12DynamicAllocation{};
    }

    void DynamicResourceHeap::ReleaseAllocatedPages()
    {
        m_GlobalDynamicAllocator.ReleasePages(m_AllocatedPages);
        m_AllocatedPages.clear();

        m_CurrOffset = InvalidOffset;
        m_AvailableSize = 0;
        m_CurrAllocatedSize = 0;
        m_CurrUsedSize = 0;
        m_CurrUsedAlignedSize = 0;
    }

} 
