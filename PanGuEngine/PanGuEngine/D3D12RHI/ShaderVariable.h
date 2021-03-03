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
    * ShaderVariableCollection持有特定类型的ShaderVariable列表。
    * PipelineState使用Manager来管理Static资源，ShaderResourceBinding使用Manager来管理Mutable和Dynamic资源
    * 把ShaderResourceLayout和ShaderResourceCache关联了起来！！！
    */
    class ShaderVariableCollection
    {
    public:
        // 为ShaderResourceLayout中的每个Shader资源创建一个ShaderVariable
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
    * 表示Shader中的一个变量，外部可以通过这个变量绑定资源(一个常量或者一张贴图)
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

        /*  只有CBV和其他Buffer的SRV/UAV可以作为Root Descriptor绑定，因为调用ID3D12GraphicsCommandList::SetGraphicsRootConstantBufferView()函数时，
        *   是不指定Buffer的大小的，Buffer的大小由Shader确定，因为Texture是需要很多信息来描述的，所以只提供一个地址是不够的
        *   Buffer直接绑定到Root Descriptor时不存在数组的情况
        *   目前只有CBV是作为Root Descriptor绑定的
        *   一个数组也用一个ShaderVariable表示，向数组中绑定资源时需要传在数组中的索引
        */
        void Set(std::shared_ptr<GpuBuffer> buffer, UINT32 arrayIndex = 0)
        {
            m_Resource.BindResource(buffer, arrayIndex, m_ParentManager.m_ResourceCache);
        }
  
        void Set(std::shared_ptr<GpuResourceView> view, UINT32 arrayIndex = 0)
        {
			m_Resource.BindResource(view, arrayIndex, m_ParentManager.m_ResourceCache);
        }


        // 是否已经绑定
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