#pragma once
#include "ShaderResource.h"
#include "ShaderResourceCache.h"
#include "GpuBuffer.h"
#include "GpuResourceDescriptor.h"

namespace RHI 
{
    struct ShaderVariableConfig;
    class RootSignature;

    /**
    * ������Shader�е���Դ��Descriptor Heap�е�ӳ��
    */
    class ShaderResourceLayout
    {
    public:
        // ����һ��Shader��������Դ����RootSignature���չ�����鵽��ͬ��Descriptor Table��ShaderResourceLayoutResource��¼Shader��Դ����Ϣ�Լ�RootIndex��OffsetFromTableStart
        ShaderResourceLayout(ID3D12Device* pd3d12Device,
                             PIPELINE_TYPE pipelineType,
                             const ShaderVariableConfig& shaderVariableConfig,
                             const ShaderResource* shaderResource,
                             RootSignature* rootSignature);

        // ��ʾShader�е�һ����Դ�����Ұ���RootIndex��OffsetFromTable����������Ϣ
        struct Resource 
        {
        public:
            Resource(const Resource&) = delete;
            Resource(Resource&&) = delete;
            Resource& operator = (const Resource&) = delete;
            Resource& operator = (Resource&&) = delete;

            // TODO:ʹ��λ��bitfield�Ż��ڴ�
            static constexpr UINT32 InvalidSamplerId = -1;
            static constexpr UINT32 InvalidRootIndex = -1;
            static constexpr UINT32 InvalidOffset = -1;

            //const ShaderResourceLayout& ParentResLayout; // ShaderResourceLayout����洢��unorderedmap�У������ÿ���ʧЧ
            const ShaderResourceAttribs& Attribs;   // ��ӦShaderResource�е�һ����Դ
            const UINT32 OffsetFromTableStart;
            const BindingResourceType ResourceType;   // CBV��TexSRV��BufSRV��TexUAV��BufUAV��Sampler
            const SHADER_RESOURCE_VARIABLE_TYPE VariableType;   // Static��Mutable��Dynamic
            const UINT32 RootIndex;

            Resource(//const ShaderResourceLayout&      _ParentLayout,
                          const ShaderResourceAttribs&     _Attribs,
                          SHADER_RESOURCE_VARIABLE_TYPE    _VariableType,
                          BindingResourceType               _ResType,
                          UINT32                           _RootIndex,
                          UINT32                           _OffsetFromTableStart) noexcept :
                //ParentResLayout{ _ParentLayout },
                Attribs{ _Attribs },
                ResourceType{ _ResType },
                VariableType{ _VariableType },
                RootIndex{ static_cast<UINT16>(_RootIndex) },
                OffsetFromTableStart{ _OffsetFromTableStart }
            {
            }

            // �Ƿ��Ѿ�������Դ
            bool IsBound(UINT32 arrayIndex, const ShaderResourceCache& resourceCache) const;

			void BindResource(std::shared_ptr<GpuBuffer> buffer, UINT32 arrayIndex, ShaderResourceCache& resourceCache) const;
			void BindResource(std::shared_ptr<GpuResourceDescriptor> descriptor, UINT32 arrayIndex, ShaderResourceCache& resourceCache) const;
        };

        UINT32 GetCbvSrvUavCount(SHADER_RESOURCE_VARIABLE_TYPE VarType) const
        {
            return m_SrvCbvUavs[VarType].size();
        }

        // indexInArray�������D3D12Resource�������е�����������RootIndex����Ϊ�����ж��RootIndex��ͬ��D3D12Resource
        const Resource& GetSrvCbvUav(SHADER_RESOURCE_VARIABLE_TYPE VarType, UINT32 indexInArray) const
        {
            return *m_SrvCbvUavs[VarType][indexInArray].get();
        }

    private:
        ID3D12Device* m_D3D12Device;

        // Shader�е�������Դ�����ո���Ƶ�ʷ�������vector
        std::vector<std::unique_ptr<Resource>> m_SrvCbvUavs[SHADER_RESOURCE_VARIABLE_TYPE_NUM_TYPES];
    };

}