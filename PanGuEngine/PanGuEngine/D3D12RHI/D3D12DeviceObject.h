#pragma once

namespace RHI 
{
    /**
    * 表示GPU对象
    */
    class D3D12DeviceObject
    {
    public:
        virtual ~D3D12DeviceObject() = 0;
    };

}