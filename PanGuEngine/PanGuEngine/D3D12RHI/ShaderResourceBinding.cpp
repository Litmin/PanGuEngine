#include "pch.h"
#include "ShaderResourceBinding.h"
#include "PipelineState.h"
#include "RenderDevice.h"
#include "ShaderResourceBindingUtility.h"

namespace RHI
{
    ShaderResourceBinding::ShaderResourceBinding(PipelineState* PSO, 
												 const SHADER_RESOURCE_VARIABLE_TYPE* allowedVarTypes,
												 UINT32 allowedTypeNum) :
        m_PSO{PSO}
    {
        auto* renderDevice = PSO->GetRenderDevice();

        // 初始化Resource Cache
        m_ShaderResourceCache.Initialize(renderDevice, m_PSO->GetRootSignature(), allowedVarTypes, allowedTypeNum);

        // lambda表达式
        m_PSO->ProcessShaders([&](SHADER_TYPE shaderType, const ShaderResourceLayout& layout)
        {
            std::unique_ptr<ShaderVariableCollection> variables = std::make_unique<ShaderVariableCollection>(&m_ShaderResourceCache,
                layout,
                allowedVarTypes,
                allowedTypeNum);
            m_ShaderVariableManagers.emplace(shaderType, std::move(variables));
        });
    }

    ShaderResourceBinding::~ShaderResourceBinding()
    {
    }
    
    ShaderVariable* ShaderResourceBinding::GetVariableByName(SHADER_TYPE ShaderType, const std::string& Name)
    {
        auto ite = m_ShaderVariableManagers.find(ShaderType);
        if (ite == m_ShaderVariableManagers.end())
            return nullptr;
    	
        return ite->second->GetVariable(Name);
    }
    
    UINT32 ShaderResourceBinding::GetVariableCount(SHADER_TYPE ShaderType) const
    {
        auto ite = m_ShaderVariableManagers.find(ShaderType);
        if (ite == m_ShaderVariableManagers.end())
            return 0;

        return ite->second->GetVariableCount();
    }
    
    ShaderVariable* ShaderResourceBinding::GetVariableByIndex(SHADER_TYPE ShaderType, UINT32 Index)
    {
        auto ite = m_ShaderVariableManagers.find(ShaderType);
        if (ite == m_ShaderVariableManagers.end())
            return nullptr;

        return ite->second->GetVariable(Index);
    }
}