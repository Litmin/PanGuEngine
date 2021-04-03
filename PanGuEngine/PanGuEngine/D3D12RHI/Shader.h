#pragma once
#include "ShaderResource.h"

namespace RHI 
{
    class RenderDevice;

    struct ShaderDesc
    {
        SHADER_TYPE ShaderType;
    };

    struct ShaderMacro
    {
        const std::string Name;
        const std::string Definition;
    };

    struct ShaderModel
    {
        UINT8 Major;
        UINT8 Minor;
    };

    struct ShaderCreateInfo
    {
        std::wstring FilePath;
        std::string entryPoint;
        ShaderMacro* Macros = nullptr;
        ShaderDesc Desc;
        ShaderModel SM = {5, 1};
    };

    /*
    * 表示一个Shader，例如一个Vertex Shader。
    * 负责编译HLSL的Shader文件。
    * Shader对象由PSO所有。
    */
    class Shader
    {
    public:
        Shader(const ShaderCreateInfo& shaderCI);

        const ShaderResource* GetShaderResources() const { return m_ShaderResource.get(); }

        ID3DBlob* GetShaderByteCode() { return m_ShaderByteCode.Get(); }

        SHADER_TYPE GetShaderType() const { return m_Desc.ShaderType; }

    private:
        ShaderDesc m_Desc;

        std::unique_ptr<const ShaderResource> m_ShaderResource;

        Microsoft::WRL::ComPtr<ID3DBlob> m_ShaderByteCode;
    };
}