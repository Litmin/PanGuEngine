#pragma once
#include "GpuResource.h"

namespace RHI 
{
    /**
    * 
    */
    class GpuResourceView
    {
    public:

    protected:
        // ����Դӵ������Ȩ��GpuResourceView����ʱ��֤��Դ�����ͷ�
        std::shared_ptr<GpuResource> m_Resource;

    };

}