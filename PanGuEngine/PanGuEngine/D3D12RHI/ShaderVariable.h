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
        ShaderVariableManager(IObject& owner,
                              ShaderResourceCache& resourceCache) noexcept :
            m_Owner{owner},
            m_ResourceCache{resourceCache}
        {
        }

        // ΪShaderResourceLayout�е�ÿ��Shader��Դ����һ��ShaderVariable
        void Initialize(ShaderResourceLayout& srcLayout,
                        const SHADER_RESOURCE_VARIABLE_TYPE* allowedVarTypes,
                        UINT32 alloedTypeNum);

        ShaderVariable* GetVariable(const char* name);
        ShaderVariable* GetVariable(UINT32 index);


        // ��������Դ
        //void BindResource(IResourceMapping* resourceMapping, UINT32 flags);

        UINT32 GetVariableCount() const { return m_Variables.size(); }


    private:
        friend ShaderVariable;

        UINT32 GetVariableIndex(const ShaderVariable& variable);

        // Owner������PSO��Ҳ������SRB
        IObject& m_Owner;

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