#pragma once
#include "ShaderResource.h"
#include "ShaderResourceCache.h"

namespace RHI 
{
    struct PipelineResourceLayoutDesc;
    class RootSignature;

    /**
    * ����Shader Resource��Descriptor Table�е�Descriptor֮��Ķ�Ӧ��ϵ
    * ������;��
    *           1��PipelineStateΪÿ��Shader����Layout
    *           2��ÿ��Shader��������Static��Դ
    *           3��ÿ��ShaderResourceBinding������������Mutable��Dynamic��Դ
    */
    class ShaderResourceLayout
    {
    public:
        ShaderResourceLayout() = default;

        // �����ֳ�ʼ����ʽ��
        //      ShaderΪ�˰�Static��Դ�����ṩһ��ShaderResourceCache
        //      SRB��Mutable��DYnamic��Դ
        // RootIndex��OffsetFromTableStart�ڳ�ʼ���Ĺ����з���
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

        // �ýṹ������Shader�е���Դ��Descriptor Heap�е�ӳ���ϵ����Ա����һ��ShaderResourceAttribs�����á�RootIndex��OffsetFromTableStart
        struct D3D12Resource 
        {
        public:
            D3D12Resource(const D3D12Resource&) = delete;
            D3D12Resource(D3D12Resource&&) = delete;
            D3D12Resource& operator = (const D3D12Resource&) = delete;
            D3D12Resource& operator = (D3D12Resource&&) = delete;

            // TODO:ʹ��λ��bitfield�Ż��ڴ�
            static constexpr const UINT32 InvalidSamplerId = -1;
            static constexpr const UINT32 InvalidRootIndex = -1;
            static constexpr const UINT32 InvalidOffset = -1;

            const ShaderResourceLayout& ParentResLayout;
            const ShaderResourceAttribs& Attribs;   // ��ӦShaderResource�е�һ����Դ
            const UINT32 OffsetFromTableStart;
            const CachedResourceType ResourceType;   // CBV��TexSRV��BufSRV��TexUAV��BufUAV��Sampler
            const SHADER_RESOURCE_VARIABLE_TYPE VariableType;   // Static��Mutable��Dynamic
            const UINT32 RootIndex;
            const UINT16 SamplerId; // ���SamplerId��ShaderResource��SamplerId��һ��

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

            // �Ƿ��Ѿ�������Դ
            bool IsBound(UINT32 arrayIndex, const ShaderResourceCache& resourceCache) const;

            void BindResource(IDeviceObject* pObject, UINT32 arrayIndex, ShaderResourceCache& resourceCache) const;


        private:
            // ��Ҫ�󶨵���Դ�洢��ShaderResourceCache��
            // ����Cache������ʵ���ǰ�һ����Դ��Descriptor������ShaderResourceCache�е�Descriptor
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

        // ShaderResource�洢��һ��Shader��Ҫ��������Դ
        std::shared_ptr<const ShaderResource> m_ShaderResources;

        // ������ӳ���ϵ
        std::vector<D3D12Resource> m_SrvCbvUavs[SHADER_RESOURCE_VARIABLE_TYPE_NUM_TYPES];
        std::vector<D3D12Resource> m_Samplers[SHADER_RESOURCE_VARIABLE_TYPE_NUM_TYPES];
    };

}