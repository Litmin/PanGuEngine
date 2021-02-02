#include "pch.h"
#include "ShaderResourceBindingUtility.h"
#include "PipelineState.h"

namespace RHI
{
    SHADER_RESOURCE_VARIABLE_TYPE GetShaderVariableType(SHADER_TYPE shaderType, 
                                                        const std::string& name, 
                                                        const ShaderVariableConfig& shaderVariableConfig)
    {
        for (UINT32 i = 0; i < shaderVariableConfig.Variables.size(); ++i)
        {
            const auto& curVarDesc = shaderVariableConfig.Variables[i];

            if (((curVarDesc.ShaderType & shaderType) != 0) && (name.compare(curVarDesc.Name) == 0))
            {
                return curVarDesc.Type;
            }

        }

        return shaderVariableConfig.DefaultVariableType;
    }
}