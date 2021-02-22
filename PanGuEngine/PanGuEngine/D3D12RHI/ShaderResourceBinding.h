#pragma once
#include "ShaderResourceCache.h"
#include "ShaderVariable.h"

namespace RHI 
{
    class PipelineState;

    /**
    * ������Դ��
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
        // һ��SRB������԰�һ������������Shader����Դ�������м���Shader���м���ShaderVariableManager��һ��ShaderVariableManager��ʾһ��Shader�ı���
        std::unordered_map<SHADER_TYPE, ShaderVariableCollection> m_ShaderVariableManagers;
    };
}