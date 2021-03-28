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

    D3D12DynamicMemoryManager::D3D12DynamicMemoryManager(UINT32 NumPagesToReserve, UINT64 PageSize)
    {
        for (UINT32 i = 0; i < NumPagesToReserve; ++i)
        {
            D3D12DynamicPage Page(PageSize);
            auto             Size = Page.GetSize();
            m_AvailablePages.emplace(Size, std::move(Page));
        }
    }

    D3D12DynamicPage D3D12DynamicMemoryManager::AllocatePage(UINT64 SizeInBytes)
    {
        auto PageIt = m_AvailablePages.lower_bound(SizeInBytes); // Returns an iterator pointing to the first element that is not less than key
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

    void D3D12DynamicMemoryManager::ReleasePages(std::vector<D3D12DynamicPage>& Pages)
    {
        struct StalePage
        {
            D3D12DynamicPage           Page;
            D3D12DynamicMemoryManager* Mgr;

            StalePage(D3D12DynamicPage&& _Page, D3D12DynamicMemoryManager& _Mgr) noexcept :
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

    void D3D12DynamicMemoryManager::Destroy()
    {
        m_AvailablePages.clear();
    }

    D3D12DynamicMemoryManager::~D3D12DynamicMemoryManager()
    {
        assert(m_AvailablePages.empty() && "Not all pages are destroyed. Dynamic memory manager must be explicitly destroyed with Destroy() method");
    }


    D3D12DynamicHeap::~D3D12DynamicHeap()
    {
        VERIFY(m_AllocatedPages.empty(), "Allocated pages have not been released which indicates FinishFrame() has not been called");

        auto PeakAllocatedPages = m_PeakAllocatedSize / m_PageSize;
        LOG_INFO_MESSAGE(m_HeapName,
            " usage stats:\n"
            "                       Peak used/aligned/allocated size: ",
            FormatMemorySize(m_PeakUsedSize, 2, m_PeakAlignedSize), " / ",
            FormatMemorySize(m_PeakAlignedSize, 2, m_PeakAlignedSize), " / ",
            FormatMemorySize(m_PeakAllocatedSize, 2, m_PeakAllocatedSize),
            " (", PeakAllocatedPages, (PeakAllocatedPages == 1 ? " page)" : " pages)"),
            ". Peak efficiency (used/aligned): ", std::fixed, std::setprecision(1), static_cast<double>(m_PeakUsedSize) / static_cast<double>(std::max(m_PeakAlignedSize, Uint64{ 1 })) * 100.0, '%',
            ". Peak utilization (used/allocated): ", std::fixed, std::setprecision(1), static_cast<double>(m_PeakUsedSize) / static_cast<double>(std::max(m_PeakAllocatedSize, Uint64{ 1 })) * 100.0, '%');
    }

    D3D12DynamicAllocation D3D12DynamicHeap::Allocate(Uint64 SizeInBytes, Uint64 Alignment, Uint64 DvpCtxFrameNumber)
    {
        VERIFY_EXPR(Alignment > 0);
        VERIFY(IsPowerOfTwo(Alignment), "Alignment (", Alignment, ") must be power of 2");

        auto align = Align(m_CurrOffset, Alignment);
        align++;

        if (m_CurrOffset == InvalidOffset || SizeInBytes + (Align(m_CurrOffset, Alignment) - m_CurrOffset) > m_AvailableSize)
        {
            auto NewPageSize = m_PageSize;
            while (NewPageSize < SizeInBytes)
                NewPageSize *= 2;

            auto NewPage = m_GlobalDynamicMemMgr.AllocatePage(NewPageSize);
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
            VERIFY_EXPR(AdjustedSize <= m_AvailableSize);
            m_AvailableSize -= AdjustedSize;
            m_CurrOffset += AdjustedSize;

            m_CurrUsedSize += SizeInBytes;
            m_PeakUsedSize = std::max(m_PeakUsedSize, m_CurrUsedSize);

            m_CurrAlignedSize += AdjustedSize;
            m_PeakAlignedSize = std::max(m_PeakAlignedSize, m_CurrAlignedSize);

            auto& CurrPage = m_AllocatedPages.back();
            // clang-format off
            return D3D12DynamicAllocation
            {
                CurrPage.GetD3D12Buffer(),
                AlignedOffset,
                SizeInBytes,
                CurrPage.GetCPUAddress(AlignedOffset),
                CurrPage.GetGPUAddress(AlignedOffset)
    #ifdef DILIGENT_DEVELOPMENT
                , DvpCtxFrameNumber
    #endif
            };
            // clang-format on
        }
        else
            return D3D12DynamicAllocation{};
    }

    void D3D12DynamicHeap::ReleaseAllocatedPages(Uint64 QueueMask)
    {
        m_GlobalDynamicMemMgr.ReleasePages(m_AllocatedPages, QueueMask);
        m_AllocatedPages.clear();

        m_CurrOffset = InvalidOffset;
        m_AvailableSize = 0;
        m_CurrAllocatedSize = 0;
        m_CurrUsedSize = 0;
        m_CurrAlignedSize = 0;
    }

} 
