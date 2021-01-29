#include "pch.h"
#include "ShaderResourceBinding.h"

namespace RHI
{
    ShaderResourceBinding::ShaderResourceBinding(PipelineState* PSO, bool IsPSOInternal)
    {

    }

    ShaderResourceBinding::~ShaderResourceBinding()
    {

    }
    
    ShaderVariable* ShaderResourceBinding::GetVariableByName(SHADER_TYPE ShaderType, const std::string& Name)
    {
        return nullptr;
    }
    
    UINT32 ShaderResourceBinding::GetVariableCount(SHADER_TYPE ShaderType) const
    {
        return UINT32();
    }
    
    ShaderVariable* ShaderResourceBinding::GetVariableByIndex(SHADER_TYPE ShaderType, UINT32 Index)
    {
        return nullptr;
    }
}