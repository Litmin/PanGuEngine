#pragma once

namespace RHI 
{
    /**
    * ����ɱ��С���ڴ�������������map���ٿ����ڴ�飬����¼��ʹ�õ��ڴ棬
    * ����map�������ã���һ��map��offset���򣬵ڶ���map���ڴ��Ĵ�С�����������롢ɾ�����ϲ����ȽϿ졣
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