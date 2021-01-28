#pragma once
#include "ShaderResourceCache.h"
#include "ShaderVariable.h"

namespace RHI 
{
    class PipelineState;

    /**
    * 
    */
    class ShaderResourceBinding
    {
    public:
        ShaderResourceBinding(PipelineState* PSO, bool IsPSOInternal);
        ~ShaderResourceBinding();


        ShaderVariable* GetVariableByName(SHADER_TYPE ShaderType, const std::string& Name);

        UINT32 GetVariableCount(SHADER_TYPE ShaderType) const;

        ShaderVariable* GetVariableByIndex(SHADER_TYPE ShaderType, UINT32 Index);

        PipelineState* GetPipelineState() { return m_PSO; }

        ShaderResourceCache& GetResourceCache() { return m_ShaderResourceCache; }


    private:
        PipelineState* m_PSO;

        ShaderResourceCache m_ShaderResourceCache;
        ShaderVariableManager* m_ShaderVariableManager = nullptr;

        std::array<INT8, (size_t)MAX_SHADERS_IN_PIPELINE> m_ShaderTypeToIndexMap = { -1,-1,-1,-1,-1 };

        bool m_bStaticResourcesInitialized = false;
        const UINT8 m_ShaderNum = 0;

    };
}