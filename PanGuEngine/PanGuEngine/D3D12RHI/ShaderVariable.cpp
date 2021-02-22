#include "pch.h"
#include "ShaderVariable.h"
#include "ShaderResourceBindingUtility.h"

using namespace std;

namespace RHI
{
    ShaderVariable* ShaderVariableCollection::GetVariable(const string& name)
    {
        for (UINT32 i = 0; i < m_Variables.size(); ++i)
        {

            if (m_Variables[i].m_Resource.Attribs.Name.compare(name) == 0)
            {
                return &m_Variables[i];
            }
        }

        return nullptr;
    }

    ShaderVariable* ShaderVariableCollection::GetVariable(UINT32 index)
    {
        if (index > 0 && index < m_Variables.size())
            return &m_Variables[index];

        return nullptr;
    }
}