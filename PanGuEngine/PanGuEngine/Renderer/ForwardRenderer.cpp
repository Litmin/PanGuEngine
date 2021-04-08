#include "pch.h"
#include "ForwardRenderer.h"
#include "VertexFactory.h"
#include "D3D12RHI/Shader.h"
#include "D3D12RHI/PipelineState.h"
#include "D3D12RHI/RenderDevice.h"

using namespace RHI;

void ForwardRenderer::Initialize()
{
	RHI::ShaderCreateInfo shaderCI;

	shaderCI.FilePath = L"Shaders\\Standard.hlsl";
	shaderCI.entryPoint = "VS";
	shaderCI.Desc.ShaderType = RHI::SHADER_TYPE_VERTEX;
	m_StandardVS = std::make_shared<RHI::Shader>(shaderCI);

	shaderCI.entryPoint = "PS";
	shaderCI.Desc.ShaderType = RHI::SHADER_TYPE_PIXEL;
	m_StandardPS = std::make_shared<RHI::Shader>(shaderCI);

	RHI::PipelineStateDesc PSODesc;
	PSODesc.PipelineType = RHI::PIPELINE_TYPE_GRAPHIC;
	PSODesc.GraphicsPipeline.VertexShader = m_StandardVS;
	PSODesc.GraphicsPipeline.PixelShader = m_StandardPS;
	PSODesc.GraphicsPipeline.GraphicPipelineState.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	PSODesc.GraphicsPipeline.GraphicPipelineState.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	PSODesc.GraphicsPipeline.GraphicPipelineState.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	PSODesc.GraphicsPipeline.GraphicPipelineState.SampleMask = UINT_MAX;
	PSODesc.GraphicsPipeline.GraphicPipelineState.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	PSODesc.GraphicsPipeline.GraphicPipelineState.NumRenderTargets = 1;
	PSODesc.GraphicsPipeline.GraphicPipelineState.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	PSODesc.GraphicsPipeline.GraphicPipelineState.SampleDesc.Count = 1;
	PSODesc.GraphicsPipeline.GraphicPipelineState.SampleDesc.Quality = 0;
	// TODO: SwapChain的Depth Buffer格式需要跟这里相同
	PSODesc.GraphicsPipeline.GraphicPipelineState.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// TODO: InputLayout
	UINT inputLayoutIndex = VertexFactory::GetSingleton().GetInputLayoutIndex(false, true, false, true, false, false, false);
	std::vector<D3D12_INPUT_ELEMENT_DESC>* inputLayoutDesc = VertexFactory::GetSingleton().GetInputElementDesc(inputLayoutIndex);
	PSODesc.GraphicsPipeline.GraphicPipelineState.InputLayout.NumElements = inputLayoutDesc->size();
	PSODesc.GraphicsPipeline.GraphicPipelineState.InputLayout.pInputElementDescs = inputLayoutDesc->data();

	// shader变量更新频率
	PSODesc.VariableConfig.Variables.push_back(RHI::ShaderResourceVariableDesc{ RHI::SHADER_TYPE_PIXEL, "BaseColorTex", RHI::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE });
	PSODesc.VariableConfig.Variables.push_back(RHI::ShaderResourceVariableDesc{ RHI::SHADER_TYPE_PIXEL, "EmissiveTex", RHI::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE });

	m_PSO = std::make_unique<RHI::PipelineState>(&RHI::RenderDevice::GetSingleton(), PSODesc);

	m_PerDrawCB = std::make_shared<RHI::GpuDynamicBuffer>(1, sizeof(PerDrawConstants));
	m_PerPassCB = std::make_shared<RHI::GpuDynamicBuffer>(1, sizeof(PerPassConstants));
	m_LightCB = std::make_shared<RHI::GpuDynamicBuffer>(1, sizeof(LightConstants));

	RHI::ShaderVariable* perDrawVariable = m_PSO->GetStaticVariableByName(RHI::SHADER_TYPE_VERTEX, "cbPerObject");
	perDrawVariable->Set(m_PerDrawCB);
	RHI::ShaderVariable* perPassVariable = m_PSO->GetStaticVariableByName(RHI::SHADER_TYPE_VERTEX, "cbPass");
	perPassVariable->Set(m_PerPassCB);
	RHI::ShaderVariable* lightVariable = m_PSO->GetStaticVariableByName(RHI::SHADER_TYPE_PIXEL, "cbLight");
	if (lightVariable != nullptr)
		lightVariable->Set(m_LightCB);
}

void ForwardRenderer::Render(SwapChain& swapChain)
{

}
