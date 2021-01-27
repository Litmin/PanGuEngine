#include "pch.h"
#include "ShaderVariable.h"
#include "ShaderResourceBindingUtility.h"

namespace RHI
{
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