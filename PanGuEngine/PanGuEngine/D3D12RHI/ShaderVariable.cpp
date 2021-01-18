#include "pch.h"
#include "ShaderVariable.h"
#include "ShaderResourceBindingUtility.h"

namespace RHI
{
    void ShaderVariableManager::Initialize(ShaderResourceLayout& srcLayout, 
                                           const SHADER_RESOURCE_VARIABLE_TYPE* allowedVarTypes, 
                                           UINT32 allowedTypeNum)
    {
        // 只为指定类型的资源创建ShaderVariable，因为PSO管理Static资源，SRB管理Mutable和Dynamic资源
        const UINT32 allowedTypeBits = GetAllowedTypeBits(allowedVarTypes, allowedTypeNum);

        for (SHADER_RESOURCE_VARIABLE_TYPE varType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;
            varType < SHADER_RESOURCE_VARIABLE_TYPE_NUM_TYPES;
            varType = static_cast<SHADER_RESOURCE_VARIABLE_TYPE>(varType + 1))
        {
            if (!IsAllowedType(varType, allowedTypeBits))
                continue;

            UINT32 resourceNum = srcLayout.GetCbvSrvUavCount(varType);
            for (UINT32 i = 0; i < resourceNum; ++i)
            {
                const auto& srcResource = srcLayout.GetCbvSrvUav(varType, i);
                m_Variables.emplace_back(*this, srcResource);
            }

        }
    }

    ShaderVariable* ShaderVariableManager::GetVariable(const char* name)
    {
        for (UINT32 i = 0; i < m_Variables.size(); ++i)
        {
            if (strcmp(m_Variables[i].m_Resource.Attribs.Name, Name) == 0)
            {
                return &m_Variables[i];
            }
        }

        return nullptr;
    }

    ShaderVariable* ShaderVariableManager::GetVariable(UINT32 index)
    {
        if (index > 0 && index < m_Variables.size())
            return &m_Variables[index];

        return nullptr;
    }
}