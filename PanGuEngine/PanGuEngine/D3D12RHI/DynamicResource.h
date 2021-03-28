#pragma once


namespace RHI
{

    // 表示一次动态的分配，记录了所属的ID3D12Resource、分配的大小以及CPU、GPU地址
    struct D3D12DynamicAllocation
    {
        D3D12DynamicAllocation() noexcept {}
        D3D12DynamicAllocation(ID3D12Resource*           pBuff,
                               UINT64                    _Offset,
                               UINT64                    _Size,
                               void*                     _CPUAddress,
                               D3D12_GPU_VIRTUAL_ADDRESS _GPUAddress
        ) noexcept :
            pBuffer{ pBuff },
            Offset{ _Offset },
            Size{ _Size },
            CPUAddress{ _CPUAddress },
            GPUAddress{ _GPUAddress }
        {}

        ID3D12Resource* pBuffer = nullptr; 
        UINT64 Offset = 0;
        UINT64 Size = 0;
        void* CPUAddress = nullptr;                     // The CPU-writeable address
        D3D12_GPU_VIRTUAL_ADDRESS GPUAddress = 0;       // The GPU-visible address
    };

    // 表示GPU上的一块内存，DynamicPage的构造函数中会在D3D的Upload堆中创建一个Buffer。要分配的资源都在这个Page中进行子分配。
    class D3D12DynamicPage
    {
    public:
        D3D12DynamicPage(UINT64 Size);

        D3D12DynamicPage(D3D12DynamicPage&&) = default;

        D3D12DynamicPage(const D3D12DynamicPage&) = delete;
        D3D12DynamicPage& operator= (const D3D12DynamicPage&) = delete;
        D3D12DynamicPage& operator= (D3D12DynamicPage&&) = delete;

        void* GetCPUAddress(UINT64 Offset)
        {
            assert(m_pd3d12Buffer != nullptr);
            assert(Offset < GetSize());
            // TODO: 为什么要转换成UINT8* ???
            return reinterpret_cast<UINT8*>(m_CPUVirtualAddress) + Offset;
        }

        D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(UINT64 Offset)
        {
            assert(m_pd3d12Buffer != nullptr);
            assert(Offset < GetSize());
            return m_GPUVirtualAddress + Offset;
        }

        ID3D12Resource* GetD3D12Buffer()
        {
            return m_pd3d12Buffer.Get();
        }

        UINT64 GetSize() const
        {
            assert(m_pd3d12Buffer != nullptr);
            return m_pd3d12Buffer->GetDesc().Width;
        }

        bool IsValid() const { return m_pd3d12Buffer != nullptr; }

    private:
        Microsoft::WRL::ComPtr<ID3D12Resource> m_pd3d12Buffer;
        void* m_CPUVirtualAddress = nullptr;                     // The CPU-writeable address
        D3D12_GPU_VIRTUAL_ADDRESS m_GPUVirtualAddress = 0;       // The GPU-visible address
    };


    // 管理所有的动态资源使用的内存
    class D3D12DynamicMemoryManager
    {
    public:
        /// <summary>
        /// 
        /// </summary>
        /// <param name="NumPagesToReserve">预先创建多少Page</param>
        /// <param name="PageSize"></param>
        D3D12DynamicMemoryManager(UINT32 NumPagesToReserve,
                                  UINT64 PageSize);
        ~D3D12DynamicMemoryManager();

        D3D12DynamicMemoryManager(const D3D12DynamicMemoryManager&) = delete;
        D3D12DynamicMemoryManager(D3D12DynamicMemoryManager&&) = delete;
        D3D12DynamicMemoryManager& operator= (const D3D12DynamicMemoryManager&) = delete;
        D3D12DynamicMemoryManager& operator= (D3D12DynamicMemoryManager&&) = delete;

        void ReleasePages(std::vector<D3D12DynamicPage>& Pages);

        void Destroy();

        D3D12DynamicPage AllocatePage(UINT64 SizeInBytes);

    private:
        std::multimap<UINT64, D3D12DynamicPage, std::less<UINT64>> m_AvailablePages;
    };


    class D3D12DynamicHeap
    {
    public:
        D3D12DynamicHeap(D3D12DynamicMemoryManager& DynamicMemMgr, std::string HeapName, UINT64 PageSize) :
            m_GlobalDynamicMemMgr{ DynamicMemMgr },
            m_HeapName{ std::move(HeapName) },
            m_PageSize{ PageSize }
        {}

        D3D12DynamicHeap(const D3D12DynamicHeap&) = delete;
        D3D12DynamicHeap(D3D12DynamicHeap&&) = delete;
        D3D12DynamicHeap& operator= (const D3D12DynamicHeap&) = delete;
        D3D12DynamicHeap& operator= (D3D12DynamicHeap&&) = delete;

        ~D3D12DynamicHeap();

        D3D12DynamicAllocation Allocate(UINT64 SizeInBytes, UINT64 Alignment, UINT64 DvpCtxFrameNumber);
        void                   ReleaseAllocatedPages(UINT64 QueueMask);

        static constexpr UINT64 InvalidOffset = static_cast<UINT64>(-1);

        size_t GetAllocatedPagesCount() const { return m_AllocatedPages.size(); }

    private:
        D3D12DynamicMemoryManager& m_GlobalDynamicMemMgr;
        const std::string          m_HeapName;

        std::vector<D3D12DynamicPage> m_AllocatedPages;

        const UINT64 m_PageSize;

        UINT64 m_CurrOffset = InvalidOffset;
        UINT64 m_AvailableSize = 0;

        UINT64 m_CurrAllocatedSize = 0;
        UINT64 m_CurrUsedSize = 0;
        UINT64 m_CurrAlignedSize = 0;
        UINT64 m_PeakAllocatedSize = 0;
        UINT64 m_PeakUsedSize = 0;
        UINT64 m_PeakAlignedSize = 0;
    };

} 
