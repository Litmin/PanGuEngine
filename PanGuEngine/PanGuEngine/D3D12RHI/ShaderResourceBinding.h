#pragma once
#include "ShaderResourceCache.h"
#include "ShaderVariable.h"

namespace RHI 
{
    class PipelineState;

    /**
    * 管理资源绑定
    */
    class ShaderResourceBinding
    {
    public:
        ShaderResourceBinding(PipelineState* PSO, 
							  const SHADER_RESOURCE_VARIABLE_TYPE* allowedVarTypes,
							  UINT32 allowedTypeNum);
        ~ShaderResourceBinding();

        ShaderVariable* GetVariableByName(SHADER_TYPE ShaderType, const std::string& Name);
        UINT32 GetVariableCount(SHADER_TYPE ShaderType) const;
        ShaderVariable* GetVariableByIndex(SHADER_TYPE ShaderType, UINT32 Index);

    private:
        PipelineState* m_PSO;

        ShaderResourceCache m_ShaderResourceCache;
        // 一个SRB对象可以绑定一个管线中所有Shader的资源，所以有几个Shader就有几个ShaderVariableManager，一个ShaderVariableManager表示一个Shader的变量
        std::unordered_map<SHADER_TYPE, ShaderVariableCollection> m_ShaderVariableManagers;
    };
}