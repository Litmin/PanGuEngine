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

        // Static��Դ������PSO��ResourceCache�У��ύ��Դʱ��Ҫ������SRB
        void InitializeStaticResources();

        bool StaticResourcesInitialized() const
        {
            return m_bStaticResourcesInitialized;
        }

    private:
        PipelineState* m_PSO;

        ShaderResourceCache m_ShaderResourceCache;
        // һ��SRB������԰�һ������������Shader����Դ������m_ShaderVariableManager��m_ShaderNum��ô���
        std::vector<ShaderVariableManager> m_ShaderVariableManagers;

        // ����ShaderTypeת�ɵ�Index�������ShaderType��m_ShaderVariableManager�����е�����
        std::array<INT8, (size_t)MAX_SHADERS_IN_PIPELINE> m_ShaderTypeToIndexMap = { -1,-1,-1,-1,-1 };

        bool m_bStaticResourcesInitialized = false;
        const UINT8 m_NumShaders = 0;

    };
}