#pragma once

namespace RHI
{

    // ��ʾһ�ζ�̬�ķ��䣬��¼��������ID3D12Resource������Ĵ�С�Լ�CPU��GPU��ַ
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

    // ��ʾGPU�ϵ�һ���ڴ棬DynamicPage�Ĺ��캯���л���D3D��Upload���д���һ��Buffer��Ҫ�������Դ�������Page�н����ӷ��䡣
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
            // TODO: ΪʲôҪת����UINT8* ???
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


    // �������еĶ�̬��Դʹ�õ��ڴ棬ȫ��ֻ��һ��
    class DynamicResourceAllocator
    {
    public:
        /// <summary>
        /// 
        /// </summary>
        /// <param name="NumPagesToReserve">Ԥ�ȴ�������Page</param>
        /// <param name="PageSize"></param>
        DynamicResourceAllocator(UINT32 NumPagesToReserve,
                                  UINT64 PageSize);
        ~DynamicResourceAllocator();

        DynamicResourceAllocator(const DynamicResourceAllocator&) = delete;
        DynamicResourceAllocator(DynamicResourceAllocator&&) = delete;
        DynamicResourceAllocator& operator= (const DynamicResourceAllocator&) = delete;
        DynamicResourceAllocator& operator= (DynamicResourceAllocator&&) = delete;

        void ReleasePages(std::vector<D3D12DynamicPage>& Pages);

        void Destroy();

        D3D12DynamicPage AllocatePage(UINT64 SizeInBytes);

    private:
        std::multimap<UINT64/*Page�Ĵ�С*/, D3D12DynamicPage, std::less<UINT64>> m_AvailablePages;
    };


    class DynamicResourceHeap
    {
    public:
        DynamicResourceHeap(DynamicResourceAllocator& DynamicMemAllocator, UINT64 PageSize) :
            m_GlobalDynamicAllocator{ DynamicMemAllocator },
            m_BasePageSize{ PageSize }
        {}

        DynamicResourceHeap(const DynamicResourceHeap&) = delete;
        DynamicResourceHeap(DynamicResourceHeap&&) = delete;
        DynamicResourceHeap& operator= (const DynamicResourceHeap&) = delete;
        DynamicResourceHeap& operator= (DynamicResourceHeap&&) = delete;

        ~DynamicResourceHeap();

        D3D12DynamicAllocation Allocate(UINT64 SizeInBytes, UINT64 Alignment);
        // ��ÿ֡��ĩβ�ѷ����Page��ӵ��ͷŶ���
        void ReleaseAllocatedPages();

        static constexpr UINT64 InvalidOffset = static_cast<UINT64>(-1);

    private:
        DynamicResourceAllocator& m_GlobalDynamicAllocator;

        std::vector<D3D12DynamicPage> m_AllocatedPages;

        // ������С��Page��С�������С��������2����
        const UINT64 m_BasePageSize;

        // �ڵ�ǰPage�з����ƫ��
        UINT64 m_CurrOffset = InvalidOffset;
        // ��ǰPageʣ�������
        UINT64 m_AvailableSize = 0;

        // �ѷ���Ĵ�С
        UINT64 m_CurrAllocatedSize = 0;
        // �Ѿ�ʹ�õĴ�С
        UINT64 m_CurrUsedSize = 0;
        // �����Ĵ�С
        UINT64 m_CurrUsedAlignedSize = 0;
        UINT64 m_PeakAllocatedSize = 0;
        // ��ֵ
        UINT64 m_PeakUsedSize = 0;
        UINT64 m_PeakAlignedSize = 0;
    };

}
