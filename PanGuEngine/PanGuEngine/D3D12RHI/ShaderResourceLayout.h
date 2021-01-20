#pragma once
#include "ShaderResource.h"
#include "ShaderResourceCache.h"

namespace RHI 
{
    struct PipelineResourceLayoutDesc;
    class RootSignature;

    /**
    * 定义Shader Resource和Descriptor Table中的Descriptor之间的对应关系
    * 三个用途：
    *           1，PipelineState为每个Shader保存Layout
    *           2，每个Shader用来管理Static资源
    *           3，每个ShaderResourceBinding对象用来管理Mutable和Dynamic资源
    */
    class ShaderResourceLayout
    {
    public:
        ShaderResourceLayout() = default;

        // 有两种初始化方式：
        //      Shader为了绑定Static资源，会提供一个ShaderResourceCache
        //      SRB绑定Mutable和DYnamic资源
        // RootIndex和OffsetFromTableStart在初始化的过程中分配
        void Initialize(ID3D12Device* pd3d12Device,
                        PIPELINE_TYPE pipelineType,
                        const PipelineResourceLayoutDesc& resourceLayout,
                        std::shared_ptr<const ShaderResource> shaderResource,
                        const SHADER_RESOURCE_VARIABLE_TYPE* allowedVarTypes,
                        UINT32 allowedTypeNum,
                        ShaderResourceCache* resourceCache,
                        RootSignature* rootSignature);

        ShaderResourceLayout(const ShaderResourceLayout&) = delete;
        ShaderResourceLayout(ShaderResourceLayout&&) = delete;
        ShaderResourceLayout& operator=(const ShaderResourceLayout&) = delete;
        ShaderResourceLayout& operator=(ShaderResourceLayout&&) = delete;

        ~ShaderResourceLayout() = default;

        // 该结构保存了Shader中的资源到Descriptor Heap中的映射关系，成员包含一个ShaderResourceAttribs的引用、RootIndex、OffsetFromTableStart
        struct D3D12Resource 
        {
        public:
            D3D12Resource(const D3D12Resource&) = delete;
            D3D12Resource(D3D12Resource&&) = delete;
            D3D12Resource& operator = (const D3D12Resource&) = delete;
            D3D12Resource& operator = (D3D12Resource&&) = delete;

            // TODO:使用位域bitfield优化内存
            static constexpr const UINT32 InvalidSamplerId = -1;
            static constexpr const UINT32 InvalidRootIndex = -1;
            static constexpr const UINT32 InvalidOffset = -1;

            const ShaderResourceLayout& ParentResLayout;
            const ShaderResourceAttribs& Attribs;   // 对应ShaderResource中的一个资源
            const UINT32 OffsetFromTableStart;
            const CachedResourceType ResourceType;   // CBV、TexSRV、BufSRV、TexUAV、BufUAV、Sampler
            const SHADER_RESOURCE_VARIABLE_TYPE VariableType;   // Static、Mutable、Dynamic
            const UINT32 RootIndex;
            const UINT16 SamplerId; // 这个SamplerId跟ShaderResource的SamplerId不一样

            D3D12Resource(const ShaderResourceLayout&      _ParentLayout,
                          const ShaderResourceAttribs&     _Attribs,
                          SHADER_RESOURCE_VARIABLE_TYPE    _VariableType,
                          CachedResourceType               _ResType,
                          UINT32                           _RootIndex,
                          UINT32                           _OffsetFromTableStart,
                          UINT32                           _SamplerId) noexcept :
                ParentResLayout{ _ParentLayout },
                Attribs{ _Attribs },
                ResourceType{ _ResType },
                VariableType{ _VariableType },
                RootIndex{ static_cast<UINT16>(_RootIndex) },
                SamplerId{ static_cast<UINT16>(_SamplerId) },
                OffsetFromTableStart{ _OffsetFromTableStart }
            {
            }

            // 是否已经绑定了资源
            bool IsBound(UINT32 arrayIndex, const ShaderResourceCache& resourceCache) const;

            void BindResource(IDeviceObject* pObject, UINT32 arrayIndex, ShaderResourceCache& resourceCache) const;


        private:
            // 把要绑定的资源存储到ShaderResourceCache中
            // 几个Cache函数其实就是把一个资源的Descriptor拷贝到ShaderResourceCache中的Descriptor
            void CacheCB(IDeviceObject* pBuffer,
                         ShaderResourceCache::Resource& dstRes,
                         UINT32 arrayIndex,
                         D3D12_CPU_DESCRIPTOR_HANDLE shaderVisibleHeapCPUDescriptorHandle) const;

            template<typename TResourceViewType,
                     typename TViewTypeEnum>
            void CacheResourceView(IDeviceObject* pView,
                ShaderResourceCache::Resource& dstRes,
                UINT32 arrayIndex,
                D3D12_CPU_DESCRIPTOR_HANDLE shaderVisibleHeapCPUDescriptorHandle,
                TViewTypeEnum expectedViewType) const;

            // TODO:Implete CacheSampler
            void CacheSampler(IDeviceObject* pSampler,
                ShaderResourceCache::Resource& DstSam,
                UINT32                              arrayIndex,
                D3D12_CPU_DESCRIPTOR_HANDLE         shaderVisibleHeapCPUDescriptorHandle) const;
        };

    private:
        ID3D12Device* m_D3D12Device;

        // ShaderResource存储了一个Shader需要的所有资源
        std::shared_ptr<const ShaderResource> m_ShaderResources;

        // 保存了映射关系
        std::vector<D3D12Resource> m_SrvCbvUavs[SHADER_RESOURCE_VARIABLE_TYPE_NUM_TYPES];
        std::vector<D3D12Resource> m_Samplers[SHADER_RESOURCE_VARIABLE_TYPE_NUM_TYPES];
    };

}