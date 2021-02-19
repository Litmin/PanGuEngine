#pragma once
#include "ShaderResourceCache.h"
#include "ShaderVariable.h"

namespace RHI 
{
    class PipelineState;

    /**
    * �洢�ϲ�󶨵�Mutable��Dynamic��Դ
    * ���ύһ��SRBʱ�����PSO��Static ��Դ��������
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
        std::unordered_map<SHADER_TYPE, ShaderVariableManager> m_ShaderVariableManagers;


        bool m_bStaticResourcesInitialized = false;
        const UINT8 m_NumShaders = 0;

    };
}