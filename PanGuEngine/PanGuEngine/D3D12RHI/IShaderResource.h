#pragma once

namespace RHI 
{
    /**
    * 表示可以绑定给Shader的资源
    */
    class IShaderResource
    {
    public:
        IShaderResource() = default;
        virtual ~IShaderResource() = default;
    };

}