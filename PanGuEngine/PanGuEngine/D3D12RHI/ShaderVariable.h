#pragma once
#include "ShaderResourceLayout.h"
#include "ShaderResourceCache.h"
#include "IShaderResource.h"
#include "GpuBuffer.h"
#include "GpuResourceView.h"

namespace RHI 
{
    class ShaderVariable;

    /*
    * ShaderVariableCollection�����ض����͵�ShaderVariable�б�
    * PipelineStateʹ��Manager������Static��Դ��ShaderResourceBindingʹ��Manager������Mutable��Dynamic��Դ
    * ��ShaderResourceLayout��ShaderResourceCache����������������
    */
    class ShaderVariableCollection
    {
    public:
        // ΪShaderResourceLayout�е�ÿ��Shader��Դ����һ��ShaderVariable
        ShaderVariableCollection(ShaderResourceCache& resourceCache,
                              const ShaderResourceLayout& srcLayout,
                              const SHADER_RESOURCE_VARIABLE_TYPE* allowedVarTypes,
                              UINT32 allowedTypeNum) :
            m_ResourceCache{resourceCache}
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
                    m_Variables.emplace_back(*this, srcResource);
                }
            }
        }
        

        ShaderVariable* GetVariable(const std::string& name);
        ShaderVariable* GetVariable(UINT32 index);
        UINT32 GetVariableCount() const { return m_Variables.size(); }


    private:
        friend ShaderVariable;

        ShaderResourceCache& m_ResourceCache;

        std::vector<ShaderVariable> m_Variables;
    };

    /**
    * ��ʾShader�е�һ���������ⲿ����ͨ�������������Դ(һ����������һ����ͼ)
    */
    class ShaderVariable
    {
    public:
        ShaderVariable(ShaderVariableCollection& parentManager,
            const ShaderResourceLayout::Resource& resource) :
            m_ParentManager{parentManager},
            m_Resource{resource}
        {
        }

        ShaderVariable(const ShaderVariable&) = delete;
        ShaderVariable(ShaderVariable&&) = delete;
        ShaderVariable& operator=(const ShaderVariable&) = delete;
        ShaderVariable& operator=(ShaderVariable&&) = delete;

        /*  ֻ��CBV������Buffer��SRV/UAV������ΪRoot Descriptor�󶨣���Ϊ����ID3D12GraphicsCommandList::SetGraphicsRootConstantBufferView()����ʱ��
        *   �ǲ�ָ��Buffer�Ĵ�С�ģ�Buffer�Ĵ�С��Shaderȷ������ΪTexture����Ҫ�ܶ���Ϣ�������ģ�����ֻ�ṩһ����ַ�ǲ�����
        *   Bufferֱ�Ӱ󶨵�Root Descriptorʱ��������������
        *   Ŀǰֻ��CBV����ΪRoot Descriptor�󶨵�
        *   һ������Ҳ��һ��ShaderVariable��ʾ���������а���Դʱ��Ҫ���������е�����
        */
        void Set(std::shared_ptr<GpuBuffer> buffer, UINT32 arrayIndex = 0)
        {
            m_Resource.BindResource(buffer, arrayIndex, m_ParentManager.m_ResourceCache);
        }
  
        void Set(std::shared_ptr<GpuResourceView> view, UINT32 arrayIndex = 0)
        {
			m_Resource.BindResource(view, arrayIndex, m_ParentManager.m_ResourceCache);
        }


        // �Ƿ��Ѿ���
        bool IsBound(UINT32 arrayIndex) const
        {
            return m_Resource.IsBound(arrayIndex, m_ParentManager.m_ResourceCache);
        }

    private:
        friend ShaderVariableCollection;

        ShaderVariableCollection& m_ParentManager;
        const ShaderResourceLayout::Resource& m_Resource;

    };
}