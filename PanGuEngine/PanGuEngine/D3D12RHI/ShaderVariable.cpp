#include "pch.h"
#include "ShaderVariable.h"
#include "ShaderResourceBindingUtility.h"

using namespace std;

namespace RHI
{

	ShaderVariableCollection::ShaderVariableCollection(ShaderResourceCache* resourceCache, 
													   const ShaderResourceLayout& srcLayout, 
													   const SHADER_RESOURCE_VARIABLE_TYPE* allowedVarTypes, 
													   UINT32 allowedTypeNum) :
		m_ResourceCache{ resourceCache }
	{
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
				const auto& srcResource = srcLayout.GetSrvCbvUav(varType, i);
				std::unique_ptr<ShaderVariable> shaderVariable = std::make_unique<ShaderVariable>(m_ResourceCache, srcResource);
				m_Variables.push_back(std::move(shaderVariable));
			}
		}
	}

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
		m_Resource.BindResource(buffer, arrayIndex, *m_ResourceCache);
	}

	void ShaderVariable::Set(std::shared_ptr<GpuResourceDescriptor> view, UINT32 arrayIndex /*= 0*/)
	{
		m_Resource.BindResource(view, arrayIndex, *m_ResourceCache);
	}

	bool ShaderVariable::IsBound(UINT32 arrayIndex) const
	{
		return m_Resource.IsBound(arrayIndex, *m_ResourceCache);
	}

}