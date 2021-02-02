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
        m_ShaderTypeToIndexMap.fill(-1);

        auto* renderDevice = PSO->GetRenderDevice();

        // 初始化Resource Cache
        PSO->GetRootSignature().InitResourceCacheForSRB(renderDevice, m_ShaderResourceCache);

        const SHADER_RESOURCE_VARIABLE_TYPE AllowedVarTypes[] = { SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE, SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC };

        // 初始化Shader Variable
        for (UINT32 i = 0; i < m_NumShaders; ++i)
        {
            const auto shaderType = PSO->GetShaderType(i);
            const auto& srcLayout = PSO->GetShaderResLayout(i);
            const auto shaderIndex = GetShaderTypePipelineIndex(shaderType, PSO->GetDesc().PipelineType);

            m_ShaderVariableManagers.emplace_back(m_ShaderResourceCache, srcLayout, AllowedVarTypes, _countof(AllowedVarTypes));

            m_ShaderTypeToIndexMap[shaderIndex] = i;
        }
    }

    ShaderResourceBinding::~ShaderResourceBinding()
    {
    }
    
    ShaderVariable* ShaderResourceBinding::GetVariableByName(SHADER_TYPE ShaderType, const std::string& Name)
    {
        INT32 shaderIndex = GetShaderTypePipelineIndex(ShaderType, m_PSO->GetDesc().PipelineType);

        return m_ShaderVariableManagers[shaderIndex].GetVariable(Name);
    }
    
    UINT32 ShaderResourceBinding::GetVariableCount(SHADER_TYPE ShaderType) const
    {
        INT32 shaderIndex = GetShaderTypePipelineIndex(ShaderType, m_PSO->GetDesc().PipelineType);

        return m_ShaderVariableManagers[shaderIndex].GetVariableCount();
    }
    
    ShaderVariable* ShaderResourceBinding::GetVariableByIndex(SHADER_TYPE ShaderType, UINT32 Index)
    {
        INT32 shaderIndex = GetShaderTypePipelineIndex(ShaderType, m_PSO->GetDesc().PipelineType);

        return m_ShaderVariableManagers[shaderIndex].GetVariable(Index);
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