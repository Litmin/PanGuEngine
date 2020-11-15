#pragma once

namespace RHI 
{
    /**
    * ����ɱ��С���ڴ�������������map���ٿ����ڴ�飬����¼��ʹ�õ��ڴ棬
    * ����map�������ã���һ��map��offset���򣬵ڶ���map���ڴ��Ĵ�С�����������롢ɾ�����ϲ����ȽϿ졣
    */
    class VariableSizeAllocationsManager
    {
    private:
        struct FreeBlockInfo;

        // ����Offset����Ŀ����ڴ��
        using TFreeBlocksByOffsetMap =
            std::map<size_t, FreeBlockInfo, std::less<size_t>>;

        // ���մ�С����Ŀ����ڴ��,����ʹ��multimap����Ϊ����ڴ��Ĵ�С������ͬ������offset������ͬ
        using TFreeBlocksBySizeMap =
            std::multimap<size_t, TFreeBlocksByOffsetMap::iterator, std::less<size_t>>;

        struct FreeBlockInfo
        {
            // ��������Ƕ���ģ������ڴ��Ĵ�С����ͨ��orderBySizeIt��ȡ
            size_t size;

            TFreeBlocksBySizeMap::iterator orderBySizeIt;;

            FreeBlockInfo(size_t _size) :
                size(_size) {}
        };

    public:
        VariableSizeAllocationsManager(size_t maxSize) :
            m_MaxSize(maxSize),
            m_FreeSize(maxSize)
        {
            // ��ʼ�������ڴ��Ϊһ�飬��С��maxSize
            AddNewBlock(0, maxSize);
            ResetCurrAlignment();
        }

        ~VariableSizeAllocationsManager()
        {

        }

        // �ƶ����캯������Ϊnoexpect���������Ͳ��������쳣����Ĵ��룬���Ч�ʣ���Ϊͨ���ƶ����캯����������ڴ棬���ᷢ���쳣
        VariableSizeAllocationsManager(VariableSizeAllocationsManager&& rhs) noexcept :
            m_FreeBlocksByOffset {std::move(rhs.m_FreeBlocksByOffset)},
            m_FreeBlocksBySize   {std::move(rhs.m_FreeBlocksBySize)},
            m_MaxSize            {rhs.m_MaxSize},
            m_FreeSize           {rhs.m_FreeSize},
            m_CurrAlignment      {rhs.m_CurrAlignment}
        {
            rhs.m_MaxSize = 0;
            rhs.m_FreeSize = 0;
            rhs.m_CurrAlignment = 0;
        }

        VariableSizeAllocationsManager& operator = (VariableSizeAllocationsManager&& rhs)  = delete;
        VariableSizeAllocationsManager(const VariableSizeAllocationsManager&)              = delete;
        VariableSizeAllocationsManager& operator = (const VariableSizeAllocationsManager&) = delete;

        // Allocate()�������ص�Offset������û����ģ�����Allocation��size����ȷ�����
        struct Allocation
        {
            Allocation(size_t offset, size_t size) :
                unalignedOffset { offset },
                size            { size }
            {}

            Allocation() {}

            static constexpr size_t InvalidOffset = static_cast<size_t>(-1);
            static Allocation           InvalidAllocation()
            {
                return Allocation{ InvalidOffset, 0 };
            }

            bool IsValid() const
            {
                return unalignedOffset != InvalidAllocation().unalignedOffset;
            }

            bool operator==(const Allocation& rhs) const
            {
                return unalignedOffset == rhs.unalignedOffset &&
                       size == rhs.size;
            }

            size_t unalignedOffset = InvalidOffset;
            size_t size = 0;
        };

        Allocation Allocate(size_t size, size_t alignment)
        {
            assert(size > 0);
            assert(IsPowerOfTwo(alignment));

            size = Align(size, alignment);
            if (m_FreeSize < size)
                return Allocation::InvalidAllocation();

            auto alignmentReserve = (alignment > m_CurrAlignment) ? alignment - m_CurrAlignment : 0;

            // ���ҵ�һ����Size + alignmentReserve��Ŀ����ڴ��
            auto smallestBlockItIt = m_FreeBlocksBySize.lower_bound(size + alignmentReserve);
            if (smallestBlockItIt == m_FreeBlocksBySize.end())
                return Allocation::InvalidAllocation();

            auto smallestBlockIt = smallestBlockItIt->second;
            assert((size + alignmentReserve) <= smallestBlockIt->second.size);

            //     SmallestBlockIt.Offset
            //        |                                  |
            //        |<------SmallestBlockIt.Size------>|
            //        |<------Size------>|<---NewSize--->|
            //        |                  |
            //      Offset              NewOffset
            //

            // ���������offset�ʹ�С
            size_t offset = smallestBlockIt->first;
            assert(offset % m_CurrAlignment == 0);
            size_t alignedOffset = Align(offset, alignment);
            size_t adjustedSize = size + (alignedOffset - offset);
            assert(adjustedSize <= size + alignmentReserve);

            size_t newOffset = offset + adjustedSize;
            size_t newSize = smallestBlockIt->second.size - adjustedSize;

            assert(smallestBlockItIt == smallestBlockIt->second.orderBySizeIt);
            m_FreeBlocksBySize.erase(smallestBlockItIt);
            m_FreeBlocksByOffset.erase(smallestBlockIt);

            if (newSize > 0)
                AddNewBlock(newOffset, newSize);

            m_FreeSize -= adjustedSize;

            if ((size & (m_CurrAlignment - 1)) != 0)
            {
                if (IsPowerOfTwo(size))
                {
                    assert(size >= alignment && size < m_CurrAlignment);
                    m_CurrAlignment = size;
                }
                else
                {
                    m_CurrAlignment = std::min(m_CurrAlignment, alignment);
                }
            }

            return Allocation(offset, adjustedSize);
        }

        void Free(Allocation&& allocation)
        {
            Free(allocation.unalignedOffset, allocation.size);
            allocation = Allocation{};
        }

        void Free(size_t offset, size_t size)
        {
            // ʹ�ð�Offset�����map������Ҫ�ͷŵ��ڴ��ĺ���һ�������ڴ��
            auto nextBlockIt = m_FreeBlocksByOffset.upper_bound(offset);

            // Ҫ�ͷŵ��ڴ�鲻�ܺ���һ�������ڴ���ص�
            assert(nextBlockIt == m_FreeBlocksByOffset.end() || offset + size <= nextBlockIt->first);

            // Ҫ�ͷŵ��ڴ���ǰһ�������ڴ��
            auto previousBlockIt = nextBlockIt;
            if (previousBlockIt != m_FreeBlocksByOffset.begin())
            {
                --previousBlockIt;
                // Ҫ�ͷŵ��ڴ�鲻�ܸ�ǰһ�������ڴ���ص�
                assert(offset >= previousBlockIt->first + previousBlockIt->second.size);
            }
            else
            {
                // Ҫ�ͷŵ��ڴ����ǵ�һ���ڴ��
                previousBlockIt = m_FreeBlocksByOffset.end();
            }

            // һ�����������������
            size_t newSize, newOffset;
            // Ҫ�ͷŵ��ڴ���ǰһ�������ڴ������
            if ((previousBlockIt != m_FreeBlocksByOffset.end()) && (offset == previousBlockIt->first + previousBlockIt->second.size))
            {
                //  PrevBlock.Offset             Offset
                //       |                          |
                //       |<-----PrevBlock.Size----->|<------Size-------->|
                //

                newSize = previousBlockIt->second.size + size;
                newOffset = previousBlockIt->first;

                // Ҫ�ͷŵ��ڴ�����һ�������ڴ��Ҳ����, �ϲ��������ڴ��
                if ((nextBlockIt != m_FreeBlocksByOffset.end()) && (offset + size == nextBlockIt->first))
                {
                    //   PrevBlock.Offset           Offset            NextBlock.Offset
                    //     |                          |                    |
                    //     |<-----PrevBlock.Size----->|<------Size-------->|<-----NextBlock.Size----->|
                    //

                    newSize += nextBlockIt->second.size;

                    // ���ͷ�OrderBySize��map����ȻOrderBySize�ĵ�������ʧЧ
                    m_FreeBlocksBySize.erase(previousBlockIt->second.orderBySizeIt);
                    m_FreeBlocksBySize.erase(nextBlockIt->second.orderBySizeIt);

                    ++nextBlockIt;
                    m_FreeBlocksByOffset.erase(previousBlockIt, nextBlockIt);
                }
                // Ҫ�ͷŵ��ڴ�鲻����һ�������ڴ�����ڣ���ǰһ���ڴ��ϲ�
                else
                {
                    //   PrevBlock.Offset           Offset                     NextBlock.Offset
                    //     |                          |                             |
                    //     |<-----PrevBlock.Size----->|<------Size-------->| ~ ~ ~  |<-----NextBlock.Size----->|
                    //

                    // ���ͷ�OrderBySize��map����ȻOrderBySize�ĵ�������ʧЧ
                    m_FreeBlocksBySize.erase(previousBlockIt->second.orderBySizeIt);
                    m_FreeBlocksByOffset.erase(previousBlockIt);
                }
            }
            // Ҫ�ͷŵ��ڴ�鲻��ǰһ�������ڴ������,���Ǻ���һ�������ڴ������, ����һ�������ڴ��ϲ�
            else if ((nextBlockIt != m_FreeBlocksByOffset.end()) && (offset + size == nextBlockIt->first))
            {
                //   PrevBlock.Offset                   Offset            NextBlock.Offset
                //     |                                  |                    |
                //     |<-----PrevBlock.Size----->| ~ ~ ~ |<------Size-------->|<-----NextBlock.Size----->|
                //
                newSize = size + nextBlockIt->second.size;
                newOffset = offset;

                // ���ͷ�OrderBySize��map����ȻOrderBySize�ĵ�������ʧЧ
                m_FreeBlocksBySize.erase(nextBlockIt->second.orderBySizeIt);
                m_FreeBlocksByOffset.erase(nextBlockIt);
            }
            // Ҫ�ͷŵ��ڴ�������������ڴ�鶼������,ֻ����һ���µĿ����ڴ��
            else
            {
                newSize = size;
                newOffset = offset;
            }

            AddNewBlock(newOffset, newSize);

            m_FreeSize += size;

            if (IsEmpty())
            {
                assert(GetFreeBlocksNum() == 1);
                ResetCurrAlignment();
            }
        }

        bool IsFull() const { return m_FreeSize == 0; }
        bool IsEmpty() const { return m_FreeSize == m_MaxSize; }
        size_t GetMaxSize() const { return m_MaxSize; }
        size_t GetFreeSize() const { return m_FreeSize; }
        size_t GetUsedSize() const { return m_MaxSize - m_FreeSize; }

        size_t GetFreeBlocksNum() const
        {
            return m_FreeBlocksByOffset.size();
        }

        void Extend(size_t extraSize)
        {
            size_t newBlockOffset = m_MaxSize;
            size_t newBlockSize = extraSize;

            // ������һ�������ڴ������ĩ�ˣ��������һ�������ڴ��ϲ�
            if (!m_FreeBlocksByOffset.empty())
            {
                auto lastBlockIt = m_FreeBlocksByOffset.end();
                --lastBlockIt;

                const auto lastBlockOffset = lastBlockIt->first;
                const auto lastBlockSize = lastBlockIt->second.size;
                if (lastBlockOffset + lastBlockSize == m_MaxSize)
                {
                    newBlockOffset = lastBlockOffset;
                    newBlockSize += lastBlockSize;

                    // ���ͷ�OrderBySize��map����ȻOrderBySize�ĵ�������ʧЧ
                    m_FreeBlocksBySize.erase(lastBlockIt->second.orderBySizeIt);
                    m_FreeBlocksByOffset.erase(lastBlockIt);
                }
            }

            AddNewBlock(newBlockOffset, newBlockSize);

            m_MaxSize += extraSize;
            m_FreeSize += extraSize;
        }

    private:
        void AddNewBlock(size_t offset, size_t size)
        {
            auto newBlockIt = m_FreeBlocksByOffset.emplace(offset, size);
            auto orderIt = m_FreeBlocksBySize.emplace(size, newBlockIt.first);
            newBlockIt.first->second.orderBySizeIt = orderIt;
        }

        void ResetCurrAlignment()
        {
            for (m_CurrAlignment = 1; m_CurrAlignment * 2 <= m_MaxSize; m_CurrAlignment *= 2)
            {
            }
        }

        TFreeBlocksByOffsetMap m_FreeBlocksByOffset;
        TFreeBlocksBySizeMap m_FreeBlocksBySize;

        size_t m_MaxSize       = 0;
        size_t m_FreeSize      = 0;
        size_t m_CurrAlignment = 0;
    };

}