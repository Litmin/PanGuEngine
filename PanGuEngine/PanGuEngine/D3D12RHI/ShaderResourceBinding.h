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

        // Static资源保存在PSO的ResourceCache中，提交资源时需要拷贝到SRB
        void InitializeStaticResources();

        bool StaticResourcesInitialized() const
        {
            return m_bStaticResourcesInitialized;
        }

    private:
        PipelineState* m_PSO;

        ShaderResourceCache m_ShaderResourceCache;
        // 一个SRB对象可以绑定一个管线中所有Shader的资源，所以m_ShaderVariableManager有m_ShaderNum这么多个
        std::vector<ShaderVariableManager> m_ShaderVariableManagers;

        // 输入ShaderType转成的Index，输出该ShaderType在m_ShaderVariableManager数组中的索引
        std::array<INT8, (size_t)MAX_SHADERS_IN_PIPELINE> m_ShaderTypeToIndexMap = { -1,-1,-1,-1,-1 };

        bool m_bStaticResourcesInitialized = false;
        const UINT8 m_NumShaders = 0;

    };
}