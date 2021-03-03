#pragma once

namespace RHI 
{
    /**
    * GPU上所有资源的基类
    * GPU资源包括：
    *   GpuBuffer：StructuredBuffer、ByteAddressBuffer、IndirectArgsBuffer、ReadbackBuffer、TypedBuffer
    *   PixelBuffer：ColorBuffer、DepthBuffer
    *   UploadBuffer
    *   Texture：
    * GpuResource继承enable_shared_from_this，因为在内部会创建GpuResourceView，GpuResourceView会拥有GpuResource的所有权，来保证使用GpuResourceView时，GpuResource不被释放
    */
    class GpuResource : std::enable_shared_from_this<GpuResource>
    {
    public:

    protected:

    };

}