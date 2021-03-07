#pragma once

namespace RHI 
{
    // Currently unused
    class RingBuffer
    {
    public:
        using OffsetType = UINT64;

        struct FrameHeadAttribs
        {
            UINT64 fenceValue;
            OffsetType offset;
            OffsetType size;
        };

        static constexpr const OffsetType InvalidOffset = static_cast<OffsetType>(-1);

        RingBuffer(OffsetType maxSize) noexcept :
            m_MaxSize{maxSize}
        {

        }

        RingBuffer(RingBuffer&& rhs) noexcept :
            m_AllocatedFrameHeads{std::move(rhs.m_AllocatedFrameHeads)},
            m_Tail{rhs.m_Tail},
            m_Head{rhs.m_Head},
            m_MaxSize{rhs.m_MaxSize},
            m_UsedSize{rhs.m_UsedSize},
            m_CurrFrameSize{rhs.m_CurrFrameSize}
        {
            rhs.m_Tail = 0;
            rhs.m_Head = 0;
            rhs.m_MaxSize = 0;
            rhs.m_UsedSize = 0;
            rhs.m_CurrFrameSize = 0;
        }

        RingBuffer& operator= (RingBuffer&& rhs) noexcept
        {
            m_AllocatedFrameHeads = std::move(rhs.m_AllocatedFrameHeads);
            m_Tail = rhs.m_Tail;
            m_Head = rhs.m_Head;
            m_MaxSize = rhs.m_MaxSize;
            m_UsedSize = rhs.m_UsedSize;
            m_CurrFrameSize = rhs.m_CurrFrameSize;

            rhs.m_MaxSize = 0;
            rhs.m_Tail = 0;
            rhs.m_Head = 0;
            rhs.m_UsedSize = 0;
            rhs.m_CurrFrameSize = 0;

            return *this;
        }

        RingBuffer(const RingBuffer&) = delete;
        RingBuffer& operator = (const RingBuffer&) = delete;

        ~RingBuffer()
        {
            assert((m_UsedSize == 0) && "All space in the ring buffer must be released");
        }

        OffsetType Allocate(OffsetType size, OffsetType alignment)
        {
            assert(size > 0);
            assert(IsPowerOfTwo(alignment));

            size = Align(size, alignment);

            // 空间不足，无法分配
            if (m_UsedSize + size > m_MaxSize)
            {
                return InvalidOffset;
            }

            auto alignedHead = Align(m_Head, alignment);
            if (m_Head > m_Tail)
            {

            }
            else if(alignedHead + size <= m_Tail)
            {


            }


        }


        OffsetType GetMaxSize() const { return m_MaxSize; }
        bool       IsFull()     const { return m_UsedSize == m_MaxSize; };
        bool       IsEmpty()    const { return m_UsedSize == 0; };
        OffsetType GetUsedSize()const { return m_UsedSize; }

    private:
        std::deque<FrameHeadAttribs> m_AllocatedFrameHeads;

        OffsetType m_Tail = 0;
        OffsetType m_Head = 0;
        OffsetType m_MaxSize = 0;
        OffsetType m_UsedSize = 0;
        OffsetType m_CurrFrameSize = 0;
    };
}