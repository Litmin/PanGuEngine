#include "pch.h"
#include "ShaderResourceBinding.h"
#include "PipelineState.h"
#include "RenderDevice.h"
#include "ShaderResourceBindingUtility.h"

namespace RHI
{
    ShaderResourceBinding::ShaderResourceBinding(PipelineState* PSO, bool IsPSOInternal) :
        m_PSO{PSO},
        m_NumShaders { static_cast<decltype(m_NumShaders)>( PSO->GetNumShaders()) }
    {
        auto* renderDevice = PSO->GetRenderDevice();

        // 初始化Resource Cache
        PSO->GetRootSignature().InitResourceCacheForSRB(renderDevice, m_ShaderResourceCache);

        const SHADER_RESOURCE_VARIABLE_TYPE AllowedVarTypes[] = { SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE, SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC };

    	// TODO:修改遍历Shader的方式
        // 初始化Shader Variable
        for (UINT32 i = 0; i < m_NumShaders; ++i)
        {
            const auto shaderType = PSO->GetShaderType(i);
            const auto& srcLayout = PSO->GetShaderResLayout(shaderType);

            m_ShaderVariableManagers.emplace(shaderType, ShaderVariableManager(m_ShaderResourceCache, 
																				srcLayout, 
																				AllowedVarTypes, 
																				_countof(AllowedVarTypes)));
        }
    }

    ShaderResourceBinding::~ShaderResourceBinding()
    {
    }
    
    ShaderVariable* ShaderResourceBinding::GetVariableByName(SHADER_TYPE ShaderType, const std::string& Name)
    {
        auto ite = m_ShaderVariableManagers.find(ShaderType);
        if (ite == m_ShaderVariableManagers.end())
            return nullptr;
    	
        return ite->second.GetVariable(Name);
    }
    
    UINT32 ShaderResourceBinding::GetVariableCount(SHADER_TYPE ShaderType) const
    {
        auto ite = m_ShaderVariableManagers.find(ShaderType);
        if (ite == m_ShaderVariableManagers.end())
            return 0;

        return ite->second.GetVariableCount();
    }
    
    ShaderVariable* ShaderResourceBinding::GetVariableByIndex(SHADER_TYPE ShaderType, UINT32 Index)
    {
        auto ite = m_ShaderVariableManagers.find(ShaderType);
        if (ite == m_ShaderVariableManagers.end())
            return nullptr;

        return ite->second.GetVariable(Index);
    }

    void ShaderResourceBinding::InitializeStaticResources()
    {
        if (StaticResourcesInitialized())
        {
            LOG_WARNING("Static resource have already been initialized in this SRB.The operation will be ignored.");

            return;
        }

        for (UINT32 i = 0; i < m_NumShaders; ++i)
        {
            const auto& ShaderResLayout = m_PSO->GetShaderResLayout(i);
            auto& StaticResLayout = m_PSO->GetStaticShaderResLayout(i);
            auto& StaticResCache = m_PSO->GetStaticShaderResCache(i);

            StaticResLayout.CopyStaticResourceDesriptorHandles(StaticResCache, ShaderResLayout, m_ShaderResourceCache);
        }

        m_bStaticResourcesInitialized = true;
    }
}