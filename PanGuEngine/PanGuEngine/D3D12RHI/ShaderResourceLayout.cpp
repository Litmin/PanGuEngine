#include "pch.h"
#include "ShaderResourceLayout.h"

namespace RHI
{
	ShaderResourceLayout::ShaderResourceLayout(ID3D12Device* pd3d12Device, 
											   PIPELINE_TYPE pipelineType, 
											   const PipelineResourceLayoutDesc& resourceLayout, 
											   std::shared_ptr<const ShaderResource> shaderResource, 
											   std::vector<SHADER_RESOURCE_VARIABLE_TYPE> variableTypes, 
											   ShaderResourceCache* resourceCache, 
											   RootSignature* rootSignature) :
		m_D3D12Device{pd3d12Device},
		m_Resources{shaderResource}
	{
		// 统计每种Descriptor的数量

		// 遍历ShaderResource的每个资源，添加到RootSignature和ShaderResourceLayout

	}
}