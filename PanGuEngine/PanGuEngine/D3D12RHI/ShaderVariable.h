#pragma once
#include "ShaderResourceLayout.h"
#include "ShaderResourceCache.h"

namespace RHI 
{
    class ShaderVariable;

    /*
    * ShaderVariableManager持有特定类型的ShaderVariable列表。
    * PipelineState使用Manager来管理Static资源，ShaderResourceBinding使用Manager来管理Mutable和Dynamic资源
    */
    class ShaderVariableManager
    {
    public:
        // 为ShaderResourceLayout中的每个Shader资源创建一个ShaderVariable
        ShaderVariableManager(ShaderResourceCache& resourceCache,
                              ShaderResourceLayout& srcLayout,
                              const SHADER_RESOURCE_VARIABLE_TYPE* allowedVarTypes,
                              UINT32 allowedTypeNum) :
            m_ResourceCache{resourceCache}
        {
            // 只为指定类型的资源创建ShaderVariable，因为PSO管理Static资源，SRB管理Mutable和Dynamic资源
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


        // 批量绑定资源
        //void BindResource(IResourceMapping* resourceMapping, UINT32 flags);

        UINT32 GetVariableCount() const { return m_Variables.size(); }


    private:
        friend ShaderVariable;

        UINT32 GetVariableIndex(const ShaderVariable& variable);

        ShaderResourceCache& m_ResourceCache;

        std::vector<ShaderVariable> m_Variables;
    };

    /**
    * 表示Shader中的一个变量，外部可以通过这个变量绑定资源(一个常量或者一张贴图)
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

        // 绑定资源!!!!!!
        void Set(IDeviceObject* object)
        {
            m_Resource.BindResource(object, 0/*Array Index*/, m_ParentManager.m_ResourceCache);
        }

        // 绑定数组资源
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

        // 是否已经绑定
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