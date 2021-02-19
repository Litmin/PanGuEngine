#pragma once
#include "ShaderResourceCache.h"
#include "ShaderVariable.h"

namespace RHI 
{
    class PipelineState;

    /**
    * 存储上层绑定的Mutable和Dynamic资源
    * 在提交一个SRB时，会把PSO的Static 资源拷贝过来
    */
    class ShaderResourceBinding
    {
    public:
        ShaderResourceBinding(PipelineState* PSO, 
							  const SHADER_RESOURCE_VARIABLE_TYPE* allowedVarTypes,
							  UINT32 allowedTypeNum);
        ~ShaderResourceBinding();


        ShaderVariable* GetVariableByName(SHADER_TYPE ShaderType, const std::string& Name);
        UINT32 GetVariableCount(SHADER_TYPE ShaderType) const;
        ShaderVariable* GetVariableByIndex(SHADER_TYPE ShaderType, UINT32 Index);

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
        std::unordered_map<SHADER_TYPE, ShaderVariableManager> m_ShaderVariableManagers;


        bool m_bStaticResourcesInitialized = false;
        const UINT8 m_NumShaders = 0;

    };
}