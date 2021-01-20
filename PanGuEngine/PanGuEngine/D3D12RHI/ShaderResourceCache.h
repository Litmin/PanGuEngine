#pragma once

namespace RHI 
{
    /**
    * 存储绑定到Shader的资源：
    * 有两种用途：
    *   1，每个Shader的ResourceLayout对象用一个Cache来存储Static资源
    *   2，每个ShaderResourceBinding对象用一个Cache来存储Mutable和Dynamic资源
    */
    class ShaderResourceCache
    {
    public:


        struct Resource
        {
            CachedResourceType Type = CachedResourceType::Unknown;
            // 该变量存储的是CPUDescriptorHeap中的Descriptor Handle
            D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptorHandle = { 0 };
            std::shared_ptr<IDeviceObject> pObject;
        };


    private:

    };
}