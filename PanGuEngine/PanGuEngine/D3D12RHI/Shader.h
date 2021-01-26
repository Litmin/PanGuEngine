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
        const std::wstring FilePath;
        const std::string entryPoint;
        const ShaderMacro* Macros;
        ShaderDesc Desc;
        ShaderModel SM = {5, 1};
    };

    class Shader
    {
    public:
        Shader(RenderDevice* pRenderDevice, const ShaderCreateInfo& shaderCI);
        ~Shader();

        const std::shared_ptr<const ShaderResource>& GetShaderResources() const { return m_ShaderResource; }

        ID3DBlob* GetShaderByteCode() { return m_ShaderByteCode.Get(); }

        SHADER_TYPE GetShaderType() const { return m_Desc.ShaderType; }

    private:
        ShaderDesc m_Desc;

        // Shader Resource对象要使用共享指针，因为ShaderResourceLayout也会引用它
        std::shared_ptr<const ShaderResource> m_ShaderResource;

        Microsoft::WRL::ComPtr<ID3DBlob> m_ShaderByteCode;
    };
}