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
        // 对资源拥有所有权，GpuResourceView存在时保证资源不被释放
        std::shared_ptr<GpuResource> m_Resource;

    };

}