#include "pch.h"
#include "ForwardRenderer.h"
#include "VertexFactory.h"
#include "D3D12RHI/Shader.h"
#include "D3D12RHI/PipelineState.h"
#include "D3D12RHI/RenderDevice.h"
#include "D3D12RHI/SwapChain.h"
#include "D3D12RHI/CommandContext.h"
#include "D3D12RHI/GpuRenderTextureDepth.h"
#include "SceneManager.h"
#include "GameObject.h"

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

	m_MainPassPSO = std::make_unique<RHI::PipelineState>(&RHI::RenderDevice::GetSingleton(), PSODesc);

	m_PerDrawCB = std::make_shared<RHI::GpuDynamicBuffer>(1, sizeof(PerDrawConstants));
	m_PerPassCB = std::make_shared<RHI::GpuDynamicBuffer>(1, sizeof(PerPassConstants));
	m_LightCB = std::make_shared<RHI::GpuDynamicBuffer>(1, sizeof(LightConstants));

	RHI::ShaderVariable* perDrawVariable = m_MainPassPSO->GetStaticVariableByName(RHI::SHADER_TYPE_VERTEX, "cbPerObject");
	perDrawVariable->Set(m_PerDrawCB);
	RHI::ShaderVariable* perPassVariable = m_MainPassPSO->GetStaticVariableByName(RHI::SHADER_TYPE_VERTEX, "cbPass");
	perPassVariable->Set(m_PerPassCB);
	RHI::ShaderVariable* lightVariable = m_MainPassPSO->GetStaticVariableByName(RHI::SHADER_TYPE_PIXEL, "cbLight");
	if (lightVariable != nullptr)
		lightVariable->Set(m_LightCB);

	// ShadowMap
	shaderCI.FilePath = L"Shaders\\ShadowMap.hlsl";
	shaderCI.entryPoint = "VS";
	shaderCI.Desc.ShaderType = RHI::SHADER_TYPE_VERTEX;
	m_ShadowMapVS = std::make_shared<RHI::Shader>(shaderCI);

	shaderCI.entryPoint = "PS";
	shaderCI.Desc.ShaderType = RHI::SHADER_TYPE_PIXEL;
	m_ShadowMapPS = std::make_shared<RHI::Shader>(shaderCI);

	PSODesc.GraphicsPipeline.VertexShader = m_ShadowMapVS;
	PSODesc.GraphicsPipeline.PixelShader = m_ShadowMapPS;
	PSODesc.GraphicsPipeline.GraphicPipelineState.NumRenderTargets = 0;
	PSODesc.GraphicsPipeline.GraphicPipelineState.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
	PSODesc.VariableConfig.Variables.clear();
	
	m_ShadowMapPSO = std::make_unique<RHI::PipelineState>(&RHI::RenderDevice::GetSingleton(), PSODesc);

	m_ShadowMap = std::make_shared<RHI::GpuRenderTextureDepth>(2048, 2048, DXGI_FORMAT_R24G8_TYPELESS);
	m_ShadowMap->SetName(L"ShadowMap");
	m_ShadowMapDSV = m_ShadowMap->CreateDSV();
	m_ShadowMapSRV = m_ShadowMap->CreateDepthSRV();

	m_ShadowMapViewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(2048), static_cast<float>(2048));
	m_ShadowMapScissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(2048), static_cast<LONG>(2048));

	m_ShadowMapPerDrawCB = std::make_shared<RHI::GpuDynamicBuffer>(1, sizeof(PerDrawConstants));
	m_ShadowMapPerPassCB = std::make_shared<RHI::GpuDynamicBuffer>(1, sizeof(PerPassConstants));

	RHI::ShaderVariable* shadowMapPerDrawVariable = m_ShadowMapPSO->GetStaticVariableByName(RHI::SHADER_TYPE_VERTEX, "cbPerObject");
	shadowMapPerDrawVariable->Set(m_ShadowMapPerDrawCB);
	RHI::ShaderVariable* shadowMapPerPassVariable = m_ShadowMapPSO->GetStaticVariableByName(RHI::SHADER_TYPE_VERTEX, "cbPass");
	shadowMapPerPassVariable->Set(m_ShadowMapPerPassCB);
}

void ForwardRenderer::Render(SwapChain& swapChain)
{
	RHI::GraphicsContext& graphicContext = RHI::GraphicsContext::Begin(L"ForwardRenderer");

	Camera* camera = SceneManager::GetSingleton().GetCamera();
	Light* light = SceneManager::GetSingleton().GetLight();
	const std::vector<MeshRenderer*>& drawList = SceneManager::GetSingleton().GetDrawList();

	// TODO: 测试是否只需要绑定一次Descriptor Heap
	ID3D12DescriptorHeap* cbvsrvuavHeap = RHI::RenderDevice::GetSingleton().GetGPUDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).GetD3D12DescriptorHeap();
	ID3D12DescriptorHeap* samplerHeap = RHI::RenderDevice::GetSingleton().GetGPUDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER).GetD3D12DescriptorHeap();
	graphicContext.SetDescriptorHeap(cbvsrvuavHeap, samplerHeap);


	// <----------------------------------ShadowMap Pass------------------------------------------->
	graphicContext.SetViewport(m_ShadowMapViewport);
	graphicContext.SetScissor(m_ShadowMapScissorRect);

	graphicContext.TransitionResource(*m_ShadowMap, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	graphicContext.ClearDepthAndStencil(*m_ShadowMapDSV);
	graphicContext.SetRenderTargets(0, nullptr, m_ShadowMapDSV.get());

	graphicContext.SetPipelineState(m_ShadowMapPSO.get());

	void* pShadowPerPassCB = m_ShadowMapPerPassCB->Map(graphicContext, 256);
	UpdateShadowPerPassCB(light, pShadowPerPassCB);

	// 渲染场景
	for (INT32 i = 0; i < drawList.size(); ++i)
	{
		void* pShadowMapPerDrawCB = m_ShadowMapPerDrawCB->Map(graphicContext, 256);
		drawList[i]->Render(graphicContext, pShadowMapPerDrawCB, false);
	}

	graphicContext.TransitionResource(*m_ShadowMap, D3D12_RESOURCE_STATE_GENERIC_READ);


	// <------------------------------------Main Pass---------------------------------------------->
	graphicContext.SetViewport(swapChain.GetViewport());
	graphicContext.SetScissor(swapChain.GetScissorRect());

	RHI::GpuResource* backBuffer = swapChain.GetCurBackBuffer();
	RHI::GpuResourceDescriptor* backBufferRTV = swapChain.GetCurBackBufferRTV();
	RHI::GpuResource* depthStencilBuffer = swapChain.GetDepthStencilBuffer();
	RHI::GpuResourceDescriptor* depthStencilBufferDSV = swapChain.GetDepthStencilDSV();

	graphicContext.TransitionResource(*backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
	graphicContext.TransitionResource(*depthStencilBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	graphicContext.ClearColor(*backBufferRTV, Colors::LightSteelBlue);
	graphicContext.ClearDepthAndStencil(*depthStencilBufferDSV);
	graphicContext.SetRenderTargets(1, &backBufferRTV, depthStencilBufferDSV);

	graphicContext.SetPipelineState(m_MainPassPSO.get());

	void* pPerPassCB = m_PerPassCB->Map(graphicContext, 256);
	camera->UpdateCameraCBs(pPerPassCB);

	void* pLightCB = m_LightCB->Map(graphicContext, 256);
	light->UpdateLightCB(pLightCB);

	// 渲染场景
	for (INT32 i = 0; i < drawList.size(); ++i)
	{
		// TODO: 优化代码结构
		drawList[i]->GetMaterial()->CreateSRB(m_MainPassPSO.get());

		void* pPerDrawCB = m_PerDrawCB->Map(graphicContext, 256);
		drawList[i]->Render(graphicContext, pPerDrawCB);
	}

	graphicContext.Finish();
}

void ForwardRenderer::UpdateShadowPerPassCB(Light* light, void* shadowPerPassCB)
{
	XMMATRIX shadowCameraView = XMMatrixMultiply(XMMatrixRotationQuaternion(light->GetGameObject()->WorldRotation()),
		XMMatrixTranslation(m_ShadowCameraPos.x, m_ShadowCameraPos.y, m_ShadowCameraPos.z));
	shadowCameraView = DirectX::XMMatrixInverse(&XMMatrixDeterminant(shadowCameraView), shadowCameraView);
	XMMATRIX shadowCameraproj = XMMatrixOrthographicLH(m_ShadowCameraWidth, m_ShadowCameraHeight, m_ShadowCameraNear, m_ShadowCameraFar);

	XMMATRIX viewProj = XMMatrixMultiply(shadowCameraView, shadowCameraproj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(shadowCameraView), shadowCameraView);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(shadowCameraproj), shadowCameraproj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	XMStoreFloat4x4(&m_ShadowMapPassCBData.View, XMMatrixTranspose(shadowCameraView));
	XMStoreFloat4x4(&m_ShadowMapPassCBData.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&m_ShadowMapPassCBData.Proj, XMMatrixTranspose(shadowCameraproj));
	XMStoreFloat4x4(&m_ShadowMapPassCBData.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&m_ShadowMapPassCBData.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&m_ShadowMapPassCBData.InvViewProj, XMMatrixTranspose(invViewProj));

	m_ShadowMapPassCBData.EyePosW = m_ShadowCameraPos;
	m_ShadowMapPassCBData.NearZ = m_ShadowCameraNear;
	m_ShadowMapPassCBData.FarZ = m_ShadowCameraFar;

	memcpy(shadowPerPassCB, &m_ShadowMapPassCBData, sizeof(PerPassConstants));
}
