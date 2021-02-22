#pragma once
#include "ShaderResource.h"
#include "ShaderResourceCache.h"
#include "IShaderResource.h"

namespace RHI 
{
    struct ShaderVariableConfig;
    class RootSignature;

    /**
    * 定义了Shader中的资源与Descriptor Heap中的映射
    */
    class ShaderResourceLayout
    {
    public:
        // 遍历一个Shader的所有资源，由RootSignature按照规则分组到不同的Descriptor Table，ShaderResourceLayoutResource记录Shader资源的信息以及RootIndex和OffsetFromTableStart
        ShaderResourceLayout(ID3D12Device* pd3d12Device,
                             PIPELINE_TYPE pipelineType,
                             const ShaderVariableConfig& shaderVariableConfig,
                             const ShaderResource* shaderResource,
                             RootSignature* rootSignature);

        // 表示Shader中的一个资源，并且包含RootIndex和OffsetFromTable两个额外信息
        struct Resource 
        {
        public:
            Resource(const Resource&) = delete;
            Resource(Resource&&) = delete;
            Resource& operator = (const Resource&) = delete;
            Resource& operator = (Resource&&) = delete;

            // TODO:使用位域bitfield优化内存
            static constexpr UINT32 InvalidSamplerId = -1;
            static constexpr UINT32 InvalidRootIndex = -1;
            static constexpr UINT32 InvalidOffset = -1;

            const ShaderResourceLayout& ParentResLayout;
            const ShaderResourceAttribs& Attribs;   // 对应ShaderResource中的一个资源
            const UINT32 OffsetFromTableStart;
            const CachedResourceType ResourceType;   // CBV、TexSRV、BufSRV、TexUAV、BufUAV、Sampler
            const SHADER_RESOURCE_VARIABLE_TYPE VariableType;   // Static、Mutable、Dynamic
            const UINT32 RootIndex;

            Resource(const ShaderResourceLayout&      _ParentLayout,
                          const ShaderResourceAttribs&     _Attribs,
                          SHADER_RESOURCE_VARIABLE_TYPE    _VariableType,
                          CachedResourceType               _ResType,
                          UINT32                           _RootIndex,
                          UINT32                           _OffsetFromTableStart) noexcept :
                ParentResLayout{ _ParentLayout },
                Attribs{ _Attribs },
                ResourceType{ _ResType },
                VariableType{ _VariableType },
                RootIndex{ static_cast<UINT16>(_RootIndex) },
                OffsetFromTableStart{ _OffsetFromTableStart }
            {
            }

            // 是否已经绑定了资源
            bool IsBound(UINT32 arrayIndex, const ShaderResourceCache& resourceCache) const;

            void BindResource(IShaderResource* pObject, UINT32 arrayIndex, ShaderResourceCache& resourceCache) const;

            SHADER_RESOURCE_VARIABLE_TYPE GetVariableType() const { return VariableType; }

        private:
            // 把要绑定的资源存储到ShaderResourceCache中
            // 几个Cache函数其实就是把一个资源的Descriptor拷贝到ShaderResourceCache中的Descriptor
            void CacheCB(IShaderResource* pBuffer,
                         ShaderResourceCache::Resource* dstRes) const;

            template<typename TResourceViewType>
            void CacheResourceView(IShaderResource* pView,
                                   ShaderResourceCache::Resource* dstRes,
                                   D3D12_CPU_DESCRIPTOR_HANDLE shaderVisibleHeapCPUDescriptorHandle) const;
        };

        UINT32 GetCbvSrvUavCount(SHADER_RESOURCE_VARIABLE_TYPE VarType) const
        {
            return m_SrvCbvUavs[VarType].size();
        }

        // indexInArray就是这个D3D12Resource在数组中的索引，不是RootIndex，因为可能有多个RootIndex相同的D3D12Resource
        const Resource& GetSrvCbvUav(SHADER_RESOURCE_VARIABLE_TYPE VarType, UINT32 indexInArray) const
        {
            return m_SrvCbvUavs[VarType][indexInArray];
        }

    private:
        ID3D12Device* m_D3D12Device;

        // Shader中的所有资源，按照更新频率分了三个vector
        std::vector<Resource> m_SrvCbvUavs[SHADER_RESOURCE_VARIABLE_TYPE_NUM_TYPES];
    };

}