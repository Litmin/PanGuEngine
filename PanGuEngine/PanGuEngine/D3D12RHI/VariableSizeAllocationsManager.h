#pragma once

namespace RHI 
{
    /**
    * 管理可变大小的内存请求，是用两个map跟踪空闲内存块，不记录已使用的内存，
    * 两个map互相引用，第一个map按offset排序，第二个map按内存块的大小排序，这样插入、删除、合并都比较快。
    */
    class VariableSizeAllocationsManager
    {
    private:
        struct FreeBlockInfo;

        // 按照Offset排序的空闲内存块
        using TFreeBlocksByOffsetMap =
            std::map<size_t, FreeBlockInfo, std::less<size_t>>;

        // 按照大小排序的空闲内存块,这里使用multimap，因为多个内存块的大小可能相同，但是offset不会相同
        using TFreeBlocksBySizeMap =
            std::multimap<size_t, TFreeBlocksByOffsetMap::iterator, std::less<size_t>>;

        struct FreeBlockInfo
        {
            // 这个变量是多余的，空闲内存块的大小可以通过orderBySizeIt获取
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
            // 初始化空闲内存块为一块，大小是maxSize
            AddNewBlock(0, maxSize);
            ResetCurrAlignment();
        }

        ~VariableSizeAllocationsManager()
        {

        }

        // 移动构造函数声明为noexpect，编译器就不会生成异常处理的代码，提高效率，因为通常移动构造函数不会分配内存，不会发生异常
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

        // Allocate()函数返回的Offset可能是没对齐的，但是Allocation的size是正确对齐的
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

            // 查找第一个比Size + alignmentReserve大的空闲内存块
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

            // 计算对齐后的offset和大小
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
            // 使用按Offset排序的map，查找要释放的内存块的后面一个空闲内存块
            auto nextBlockIt = m_FreeBlocksByOffset.upper_bound(offset);

            // 要释放的内存块不能和下一个空闲内存块重叠
            assert(nextBlockIt == m_FreeBlocksByOffset.end() || offset + size <= nextBlockIt->first);

            // 要释放的内存块的前一个空闲内存块
            auto previousBlockIt = nextBlockIt;
            if (previousBlockIt != m_FreeBlocksByOffset.begin())
            {
                --previousBlockIt;
                // 要释放的内存块不能跟前一个空闲内存块重叠
                assert(offset >= previousBlockIt->first + previousBlockIt->second.size);
            }
            else
            {
                // 要释放的内存块就是第一个内存块
                previousBlockIt = m_FreeBlocksByOffset.end();
            }

            // 一共有如下四种情况：
            size_t newSize, newOffset;
            // 要释放的内存块和前一个空闲内存块相邻
            if ((previousBlockIt != m_FreeBlocksByOffset.end()) && (offset == previousBlockIt->first + previousBlockIt->second.size))
            {
                //  PrevBlock.Offset             Offset
                //       |                          |
                //       |<-----PrevBlock.Size----->|<------Size-------->|
                //

                newSize = previousBlockIt->second.size + size;
                newOffset = previousBlockIt->first;

                // 要释放的内存块和下一个空闲内存块也相邻, 合并这三个内存块
                if ((nextBlockIt != m_FreeBlocksByOffset.end()) && (offset + size == nextBlockIt->first))
                {
                    //   PrevBlock.Offset           Offset            NextBlock.Offset
                    //     |                          |                    |
                    //     |<-----PrevBlock.Size----->|<------Size-------->|<-----NextBlock.Size----->|
                    //

                    newSize += nextBlockIt->second.size;

                    // 先释放OrderBySize的map，不然OrderBySize的迭代器会失效
                    m_FreeBlocksBySize.erase(previousBlockIt->second.orderBySizeIt);
                    m_FreeBlocksBySize.erase(nextBlockIt->second.orderBySizeIt);

                    ++nextBlockIt;
                    m_FreeBlocksByOffset.erase(previousBlockIt, nextBlockIt);
                }
                // 要释放的内存块不和下一个空闲内存块相邻，与前一个内存块合并
                else
                {
                    //   PrevBlock.Offset           Offset                     NextBlock.Offset
                    //     |                          |                             |
                    //     |<-----PrevBlock.Size----->|<------Size-------->| ~ ~ ~  |<-----NextBlock.Size----->|
                    //

                    // 先释放OrderBySize的map，不然OrderBySize的迭代器会失效
                    m_FreeBlocksBySize.erase(previousBlockIt->second.orderBySizeIt);
                    m_FreeBlocksByOffset.erase(previousBlockIt);
                }
            }
            // 要释放的内存块不和前一个空闲内存块相邻,但是和下一个空闲内存块相邻, 与下一个空闲内存块合并
            else if ((nextBlockIt != m_FreeBlocksByOffset.end()) && (offset + size == nextBlockIt->first))
            {
                //   PrevBlock.Offset                   Offset            NextBlock.Offset
                //     |                                  |                    |
                //     |<-----PrevBlock.Size----->| ~ ~ ~ |<------Size-------->|<-----NextBlock.Size----->|
                //
                newSize = size + nextBlockIt->second.size;
                newOffset = offset;

                // 先释放OrderBySize的map，不然OrderBySize的迭代器会失效
                m_FreeBlocksBySize.erase(nextBlockIt->second.orderBySizeIt);
                m_FreeBlocksByOffset.erase(nextBlockIt);
            }
            // 要释放的内存块与两个空闲内存块都不相邻,只插入一个新的空闲内存块
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

            // 如果最后一个空闲内存块在最末端，就与最后一个空闲内存块合并
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

                    // 先释放OrderBySize的map，不然OrderBySize的迭代器会失效
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