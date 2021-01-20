#pragma once

namespace RHI 
{
    /**
    * �洢�󶨵�Shader����Դ��
    * ��������;��
    *   1��ÿ��Shader��ResourceLayout������һ��Cache���洢Static��Դ
    *   2��ÿ��ShaderResourceBinding������һ��Cache���洢Mutable��Dynamic��Դ
    */
    class ShaderResourceCache
    {
    public:


        struct Resource
        {
            CachedResourceType Type = CachedResourceType::Unknown;
            // �ñ����洢����CPUDescriptorHeap�е�Descriptor Handle
            D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptorHandle = { 0 };
            std::shared_ptr<IDeviceObject> pObject;
        };


    private:

    };
}