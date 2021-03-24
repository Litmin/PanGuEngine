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

            if (m_Variables[i]->m_Resource.Attribs.Name.compare(name) == 0)
            {
                return m_Variables[i].get();
            }
        }

        return nullptr;
    }

    ShaderVariable* ShaderVariableCollection::GetVariable(UINT32 index)
    {
        if (index > 0 && index < m_Variables.size())
            return m_Variables[index].get();

        return nullptr;
    }

	void ShaderVariable::Set(std::shared_ptr<GpuBuffer> buffer, UINT32 arrayIndex /*= 0*/)
	{
		m_Resource.BindResource(buffer, arrayIndex, m_ParentManager.m_ResourceCache);
	}

	void ShaderVariable::Set(std::shared_ptr<GpuResourceDescriptor> view, UINT32 arrayIndex /*= 0*/)
	{
		m_Resource.BindResource(view, arrayIndex, m_ParentManager.m_ResourceCache);
	}

	bool ShaderVariable::IsBound(UINT32 arrayIndex) const
	{
		return m_Resource.IsBound(arrayIndex, m_ParentManager.m_ResourceCache);
	}

}