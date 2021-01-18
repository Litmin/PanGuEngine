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