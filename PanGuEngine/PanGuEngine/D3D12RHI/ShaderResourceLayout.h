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
        ShaderResourceLayout();

        void Initialize(ID3D12Device* pd3d12Device,
                        PIPELINE_TYPE pipelineType,
                        const PipelineResourceLayoutDesc& resourceLayout,
                        std::shared_ptr<const ShaderResource> shaderResource,
                        std::vector<SHADER_RESOURCE_VARIABLE_TYPE> variableTypes,
                        ShaderResourceCache* resourceCache,
                        RootSignature* rootSignature);

        ShaderResourceLayout(const ShaderResourceLayout&) = delete;
        ShaderResourceLayout(ShaderResourceLayout&&) = delete;
        ShaderResourceLayout& operator=(const ShaderResourceLayout&) = delete;
        ShaderResourceLayout& operator=(ShaderResourceLayout&&) = delete;

        ~ShaderResourceLayout();

        struct D3D12Resource final 
        {

        };

    private:
        ID3D12Device* m_D3D12Device;

        std::shared_ptr<const ShaderResource> m_Resources;

    };
}