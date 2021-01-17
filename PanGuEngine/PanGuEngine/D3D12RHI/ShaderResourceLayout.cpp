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
		// ͳ��ÿ��Descriptor������

		// ����ShaderResource��ÿ����Դ����ӵ�RootSignature��ShaderResourceLayout

	}
}