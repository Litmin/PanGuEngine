#pragma once

namespace RHI 
{
    /**
    * 管理可变大小的内存请求，是用两个map跟踪空闲内存块，不记录已使用的内存，
    * 两个map互相引用，第一个map按offset排序，第二个map按内存块的大小排序，这样插入、删除、合并都比较快。
    */
    class VariableSizeAllocationsManager
    {
    public:
        VariableSizeAllocationsManager(size_t maxSize)
        {
        
        }


    private:
        size_t m_MaxSize       = 0;
        size_t m_FreeSize      = 0;
        size_t m_CurrAlighment = 0;
    };

}