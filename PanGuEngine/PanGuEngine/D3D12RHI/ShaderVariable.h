#pragma once
#include "ShaderResourceLayout.h"
#include "ShaderResourceCache.h"

namespace RHI 
{
    class ShaderVariable;

    /*
    * ShaderVariableManager�����ض����͵�ShaderVariable�б�
    * PipelineStateʹ��Manager������Static��Դ��ShaderResourceBindingʹ��Manager������Mutable��Dynamic��Դ
    */
    class ShaderVariableManager
    {
    public:
        // ΪShaderResourceLayout�е�ÿ��Shader��Դ����һ��ShaderVariable
        ShaderVariableManager(ShaderResourceCache& resourceCache,
                              ShaderResourceLayout& srcLayout,
                              const SHADER_RESOURCE_VARIABLE_TYPE* allowedVarTypes,
                              UINT32 allowedTypeNum) :
            m_ResourceCache{resourceCache}
        {
            // ֻΪָ�����͵���Դ����ShaderVariable����ΪPSO����Static��Դ��SRB����Mutable��Dynamic��Դ
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
                    const auto& srcResource = srcLayout.GetCbvSrvUav(varType, i);
                    m_Variables.emplace_back(*this, srcResource);
                }

            }
        }
        

        ShaderVariable* GetVariable(const char* name);
        ShaderVariable* GetVariable(UINT32 index);


        // ��������Դ
        //void BindResource(IResourceMapping* resourceMapping, UINT32 flags);

        UINT32 GetVariableCount() const { return m_Variables.size(); }


    private:
        friend ShaderVariable;

        UINT32 GetVariableIndex(const ShaderVariable& variable);

        ShaderResourceCache& m_ResourceCache;

        std::vector<ShaderVariable> m_Variables;
    };

    /**
    * ��ʾShader�е�һ���������ⲿ����ͨ�������������Դ(һ����������һ����ͼ)
    */
    class ShaderVariable
    {
    public:
        ShaderVariable(ShaderVariableManager& parentManager,
            const ShaderResourceLayout::D3D12Resource& resource) :
            m_ParentManager{parentManager},
            m_Resource{resource}
        {

        }

        ShaderVariable(const ShaderVariable&) = delete;
        ShaderVariable(ShaderVariable&&) = delete;
        ShaderVariable& operator=(const ShaderVariable&) = delete;
        ShaderVariable& operator=(ShaderVariable&&) = delete;

        SHADER_RESOURCE_VARIABLE_TYPE GetType() const
        {
            return m_Resource.GetVariableType();
        }

        // ����Դ!!!!!!
        void Set(IDeviceObject* object)
        {
            m_Resource.BindResource(object, 0/*Array Index*/, m_ParentManager.m_ResourceCache);
        }

        // ��������Դ
        void SetArray(IDeviceObject* const* ppObjects, UINT32 firstElement, UINT32 elementsNum)
        {
            for (UINT32 i = 0; i < elementsNum; ++i)
            {
                m_Resource.BindResource(ppObjects[i], firstElement + i, m_ParentManager.m_ResourceCache);
            }
        }

        UINT32 GetIndex() const
        {
            return m_ParentManager.GetVariableIndex(*this);
        }

        // �Ƿ��Ѿ���
        bool IsBound(UINT32 arrayIndex) const
        {
            return m_Resource.IsBound(arrayIndex, m_ParentManager.m_ResourceCache);
        }

        const ShaderResourceLayout::D3D12Resource& GetResource() const
        {
            return m_Resource;
        }

    private:
        friend ShaderVariableManager;

        ShaderVariableManager& m_ParentManager;
        const ShaderResourceLayout::D3D12Resource& m_Resource;

    };
}