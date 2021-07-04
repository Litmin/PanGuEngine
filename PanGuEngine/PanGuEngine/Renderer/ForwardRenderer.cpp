#include "pch.h"
#include "ForwardRenderer.h"
#include "VertexFactory.h"
#include "D3D12RHI/Shader.h"
#include "D3D12RHI/PipelineState.h"
#include "D3D12RHI/RenderDevice.h"
#include "D3D12RHI/SwapChain.h"
#include "D3D12RHI/CommandContext.h"
#include "D3D12RHI/GpuRenderTextureDepth.h"
#include "D3D12RHI/GpuRenderTextureColor.h"
#include "D3D12RHI/GpuRenderTextureCube.h"
#include "SceneManager.h"
#include "GameObject.h"
#include "D3D12RHI/GpuCubemap.h"
#include "Utility/DDSTextureLoader.h"
#include "Utility/GeometryFactory.h"
#include "Mesh.h"

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
	PSODesc.VariableConfig.Variables.push_back(RHI::ShaderResourceVariableDesc{ RHI::SHADER_TYPE_PIXEL, "MetallicRoughnessTex", RHI::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE });
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
	// Bias设置
	PSODesc.GraphicsPipeline.GraphicPipelineState.RasterizerState.DepthBias = 100000;
	PSODesc.GraphicsPipeline.GraphicPipelineState.RasterizerState.DepthBiasClamp = 0.0f;
	PSODesc.GraphicsPipeline.GraphicPipelineState.RasterizerState.SlopeScaledDepthBias = 1.0f;

	m_ShadowMapPSO = std::make_unique<RHI::PipelineState>(&RHI::RenderDevice::GetSingleton(), PSODesc);

	m_ShadowMap = std::make_shared<RHI::GpuRenderTextureDepth>(m_ShadowMapSize, m_ShadowMapSize, DXGI_FORMAT_R24G8_TYPELESS);
	m_ShadowMap->SetName(L"ShadowMap");
	m_ShadowMapDSV = m_ShadowMap->CreateDSV();
	m_ShadowMapSRV = m_ShadowMap->CreateDepthSRV();

	m_ShadowMapViewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_ShadowMapSize), static_cast<float>(m_ShadowMapSize));
	m_ShadowMapScissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(m_ShadowMapSize), static_cast<LONG>(m_ShadowMapSize));

	m_ShadowMapPerDrawCB = std::make_shared<RHI::GpuDynamicBuffer>(1, sizeof(PerDrawConstants));
	m_ShadowMapPerPassCB = std::make_shared<RHI::GpuDynamicBuffer>(1, sizeof(PerPassConstants));

	RHI::ShaderVariable* shadowMapPerDrawVariable = m_ShadowMapPSO->GetStaticVariableByName(RHI::SHADER_TYPE_VERTEX, "cbPerObject");
	shadowMapPerDrawVariable->Set(m_ShadowMapPerDrawCB);
	RHI::ShaderVariable* shadowMapPerPassVariable = m_ShadowMapPSO->GetStaticVariableByName(RHI::SHADER_TYPE_VERTEX, "cbPass");
	shadowMapPerPassVariable->Set(m_ShadowMapPerPassCB);

	// 绑定ShadowMap
	RHI::ShaderVariable* shadowMapVariable = m_MainPassPSO->GetStaticVariableByName(RHI::SHADER_TYPE_PIXEL, "ShadowMap");
	if (shadowMapVariable != nullptr)
		shadowMapVariable->Set(m_ShadowMapSRV);

	// Skybox
	std::shared_ptr<RHI::GpuCubemap> SkyboxTex = DirectX::CreateCubemapFromDDSPanGu(L"Resources/Textures/StarSkybox.dds");
	m_SkyboxSRV = SkyboxTex->CreateSRV();

	shaderCI.FilePath = L"Shaders\\Skybox.hlsl";
	shaderCI.entryPoint = "VS";
	shaderCI.Desc.ShaderType = RHI::SHADER_TYPE_VERTEX;
	m_SkyboxVS = std::make_shared<RHI::Shader>(shaderCI);

	shaderCI.entryPoint = "PS";
	shaderCI.Desc.ShaderType = RHI::SHADER_TYPE_PIXEL;
	m_SkyboxPS = std::make_shared<RHI::Shader>(shaderCI);

	PSODesc.GraphicsPipeline.VertexShader = m_SkyboxVS;
	PSODesc.GraphicsPipeline.PixelShader = m_SkyboxPS;
	PSODesc.GraphicsPipeline.GraphicPipelineState.NumRenderTargets = 1;
	PSODesc.GraphicsPipeline.GraphicPipelineState.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	PSODesc.GraphicsPipeline.GraphicPipelineState.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	PSODesc.GraphicsPipeline.GraphicPipelineState.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	PSODesc.VariableConfig.Variables.clear();

	m_SkyboxPSO = std::make_unique<RHI::PipelineState>(&RHI::RenderDevice::GetSingleton(), PSODesc);

	//RHI::ShaderVariable* perDrawVariable2 = m_SkyboxPSO->GetStaticVariableByName(RHI::SHADER_TYPE_VERTEX, "cbPerObject");
	//perDrawVariable2->Set(m_PerDrawCB);
	RHI::ShaderVariable* perPassVariable2 = m_SkyboxPSO->GetStaticVariableByName(RHI::SHADER_TYPE_VERTEX, "cbPass");
	perPassVariable2->Set(m_PerPassCB);
	RHI::ShaderVariable* skyboxVariable = m_SkyboxPSO->GetStaticVariableByName(RHI::SHADER_TYPE_PIXEL, "Skybox");
	//if (skyboxVariable != nullptr)
	//	skyboxVariable->Set(m_SkyboxSRV);

	m_SkyboxMesh = GeometryFactory::CreateSphere(0.5f, 20, 20);

	// IBL
	m_CubeMesh = GeometryFactory::CreateBox(1, 1, 1, 0);

	// ----------------irradiance map-----------------------------
	shaderCI.FilePath = L"Shaders\\PreComputeIrradianceMap.hlsl";
	shaderCI.entryPoint = "VS";
	shaderCI.Desc.ShaderType = RHI::SHADER_TYPE_VERTEX;
	m_PrecomputeIrradianceVS = std::make_shared<RHI::Shader>(shaderCI);

	shaderCI.entryPoint = "PS";
	shaderCI.Desc.ShaderType = RHI::SHADER_TYPE_PIXEL;
	m_PrecomputeIrradiancePS = std::make_shared<RHI::Shader>(shaderCI);

	PSODesc.GraphicsPipeline.VertexShader = m_PrecomputeIrradianceVS;
	PSODesc.GraphicsPipeline.PixelShader = m_PrecomputeIrradiancePS;
	PSODesc.GraphicsPipeline.GraphicPipelineState.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	PSODesc.GraphicsPipeline.GraphicPipelineState.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	// 关闭深度和模板测试
	PSODesc.GraphicsPipeline.GraphicPipelineState.DepthStencilState.DepthEnable = false;
	PSODesc.GraphicsPipeline.GraphicPipelineState.DepthStencilState.StencilEnable = false;
	PSODesc.GraphicsPipeline.GraphicPipelineState.NumRenderTargets = 1;
	PSODesc.GraphicsPipeline.GraphicPipelineState.RTVFormats[0] = IrradianceCubeFmt;
	PSODesc.GraphicsPipeline.GraphicPipelineState.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	PSODesc.VariableConfig.Variables.clear();

	m_PrecomputeIrradianceMapPSO = std::make_unique<RHI::PipelineState>(&RHI::RenderDevice::GetSingleton(), PSODesc);
	RHI::ShaderVariable* irradianceMapEnvVariable = m_PrecomputeIrradianceMapPSO->GetStaticVariableByName(RHI::SHADER_TYPE_PIXEL, "EnvironmentMap");
	if(irradianceMapEnvVariable != nullptr)
		irradianceMapEnvVariable->Set(m_SkyboxSRV);

	// Mipmap填0，会自动计算有多少个Mipmap
	m_IrradianceMap = std::make_shared<RHI::GpuRenderTextureCube>(IrradianceCubeDim, IrradianceCubeDim, 0, IrradianceCubeFmt, Color(0.0f, 0.0f, 0.0f, 0.0f));
	m_IrradianceMapSRV = m_IrradianceMap->CreateSRV();
	// ----------------irradiance map-----------------------------

	// ----------------Prefilter Environment map-----------------------------
	shaderCI.FilePath = L"Shaders\\PrefilterEnvironmentMap.hlsl";
	shaderCI.entryPoint = "VS";
	shaderCI.Desc.ShaderType = RHI::SHADER_TYPE_VERTEX;
	m_PrefilterEnvVS = std::make_shared<RHI::Shader>(shaderCI);

	shaderCI.entryPoint = "PS";
	shaderCI.Desc.ShaderType = RHI::SHADER_TYPE_PIXEL;
	m_PrefilterEnvPS = std::make_shared<RHI::Shader>(shaderCI);

	PSODesc.GraphicsPipeline.VertexShader = m_PrefilterEnvVS;
	PSODesc.GraphicsPipeline.PixelShader = m_PrefilterEnvPS;
	PSODesc.GraphicsPipeline.GraphicPipelineState.NumRenderTargets = 1;
	PSODesc.GraphicsPipeline.GraphicPipelineState.RTVFormats[0] = PrefilteredEnvMapFmt;
	PSODesc.VariableConfig.Variables.clear();

	m_PrefilterEnvPSO = std::make_unique<RHI::PipelineState>(&RHI::RenderDevice::GetSingleton(), PSODesc);
	RHI::ShaderVariable* prefilterEnvVariable = m_PrefilterEnvPSO->GetStaticVariableByName(RHI::SHADER_TYPE_PIXEL, "EnvironmentMap");
	prefilterEnvVariable->Set(m_SkyboxSRV);

	m_PrefilterEnvMap = std::make_shared<RHI::GpuRenderTextureCube>(PrefilteredEnvMapDim, PrefilteredEnvMapDim, 0, PrefilteredEnvMapFmt, Color(0.0f, 0.0f, 0.0f, 0.0f));
	m_PrefilterEnvSRV = m_PrefilterEnvMap->CreateSRV();
	// ----------------Prefilter Environment map-----------------------------

	// ----------------BRDF-----------------------------
	m_BRDF_Lut = std::make_shared<RHI::GpuRenderTextureColor>(BRDF_LUT_Dim, BRDF_LUT_Dim, D3D12_RESOURCE_DIMENSION_TEXTURE2D, PrecomputeBRDFFmt);
	m_BRDF_Lut_SRV = m_BRDF_Lut->CreateSRV();

	shaderCI.FilePath = L"Shaders\\PreComputeBRDF.hlsl";
	shaderCI.entryPoint = "VS";
	shaderCI.Desc.ShaderType = RHI::SHADER_TYPE_VERTEX;
	m_PrecomputeBRDFVS = std::make_shared<RHI::Shader>(shaderCI);

	shaderCI.entryPoint = "PS";
	shaderCI.Desc.ShaderType = RHI::SHADER_TYPE_PIXEL;
	m_PrecomputeBRDFPS = std::make_shared<RHI::Shader>(shaderCI);

	PSODesc.GraphicsPipeline.VertexShader = m_PrecomputeBRDFVS;
	PSODesc.GraphicsPipeline.PixelShader = m_PrecomputeBRDFPS;
	PSODesc.GraphicsPipeline.GraphicPipelineState.NumRenderTargets = 1;
	PSODesc.GraphicsPipeline.GraphicPipelineState.RTVFormats[0] = PrecomputeBRDFFmt;
	PSODesc.VariableConfig.Variables.clear();
	m_PrecomputeBRDFPSO = std::make_unique<RHI::PipelineState>(&RHI::RenderDevice::GetSingleton(), PSODesc);


	// ----------------BRDF-----------------------------

	ComputeIrradianceMapAndFilterEnvMap();
	PrecomputeBRDF();


	skyboxVariable->Set(m_PrefilterEnvSRV);
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
	UpdateShadowPerPassCB(light, camera, pShadowPerPassCB);

	// 渲染场景
	for (INT32 i = 0; i < drawList.size(); ++i)
	{
		void* pShadowMapPerDrawCB = m_ShadowMapPerDrawCB->Map(graphicContext, 256);
		drawList[i]->Render(graphicContext, pShadowMapPerDrawCB, false);
	}

	// Shadow Map过度到Read状态
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
	camera->UpdateCameraCBs(pPerPassCB, m_ShadowMapPassCBData.ViewProj);

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

	// Skybox
	graphicContext.SetPipelineState(m_SkyboxPSO.get());

	graphicContext.SetVertexBuffer(0, m_SkyboxMesh->VertexBufferView());
	graphicContext.SetIndexBuffer(m_SkyboxMesh->IndexBufferView());

	graphicContext.DrawIndexedInstanced(m_SkyboxMesh->IndexCount(), 1, 0, 0, 0);

	graphicContext.Finish();
}

void ForwardRenderer::UpdateShadowPerPassCB(Light* light, Camera* sceneCamera, void* shadowPerPassCB)
{
	// 首先假设光源相机在原点
	XMMATRIX lightToWorld = XMMatrixRotationQuaternion(light->GetGameObject()->WorldRotation());
	XMMATRIX worldToLight = DirectX::XMMatrixInverse(&XMMatrixDeterminant(lightToWorld), lightToWorld);

	// 场景相机视锥体的八个顶点
	std::array<DirectX::XMVECTOR, 8> sceneCameraCorners = sceneCamera->GetFrustumCorners(30.0f);
	std::array<DirectX::XMFLOAT3, 8> sceneCameraCornersFloat3;
	// 变换到位于原点的光源相机空间
	for (INT32 i = 0; i < sceneCameraCorners.size(); ++i)
	{
		sceneCameraCorners[i] = DirectX::XMVector4Transform(sceneCameraCorners[i], worldToLight);
		DirectX::XMStoreFloat3(&sceneCameraCornersFloat3[i], sceneCameraCorners[i]);
	}
	// 求出光源相机视锥体的包围盒
	float minX, maxX, minY, maxY, minZ, maxZ;
	minX = maxX = sceneCameraCornersFloat3[0].x;
	minY = maxY = sceneCameraCornersFloat3[0].y;
	minZ = maxZ = sceneCameraCornersFloat3[0].z;
	for (INT32 i = 0; i < sceneCameraCornersFloat3.size(); ++i)
	{
		minX = Math::Min(minX, sceneCameraCornersFloat3[i].x);
		minY = Math::Min(minY, sceneCameraCornersFloat3[i].y);
		minZ = Math::Min(minZ, sceneCameraCornersFloat3[i].z);
		maxX = Math::Max(maxX, sceneCameraCornersFloat3[i].x);
		maxY = Math::Max(maxY, sceneCameraCornersFloat3[i].y);
		maxZ = Math::Max(maxZ, sceneCameraCornersFloat3[i].z);
	}

	// 计算出视锥体，由此构造光源相机的投影矩阵
	float maxLength = Math::Length(Math::Vector3(sceneCameraCornersFloat3[0].x - sceneCameraCornersFloat3[6].x,
												 sceneCameraCornersFloat3[0].y - sceneCameraCornersFloat3[6].y,
												 sceneCameraCornersFloat3[0].z - sceneCameraCornersFloat3[6].z)) * 1.0f;
	float orthoWidth = maxLength;
	float orthoHeight = maxLength;
	float nearPlane = 0.0f;
	float farPlane = maxZ - minZ;

	// 计算出近平面的中心点，变换到世界空间就是光源相机的位置
	DirectX::XMFLOAT3 nearPlaneCenter = { (minX + maxX) * 0.5f, (minY + maxY) * 0.5f, minZ };

	// 让光源相机每次移动整数倍的纹素,也就是让光源相机位置是worldUnitsPertexel的整数倍
	float WorldUnitsPerTexel = maxLength / m_ShadowMapSize;	// 一个纹素是多少Unit
	nearPlaneCenter.x /= WorldUnitsPerTexel;
	nearPlaneCenter.x = Math::Floor(nearPlaneCenter.x);
	nearPlaneCenter.x *= WorldUnitsPerTexel;
	nearPlaneCenter.y /= WorldUnitsPerTexel;
	nearPlaneCenter.y = Math::Floor(nearPlaneCenter.y);
	nearPlaneCenter.y *= WorldUnitsPerTexel;

	DirectX::XMFLOAT3 lightCameraPos;
	DirectX::XMStoreFloat3(&lightCameraPos, DirectX::XMVector4Transform(DirectX::XMLoadFloat3(&nearPlaneCenter), lightToWorld));


	XMMATRIX shadowCameraView = XMMatrixMultiply(XMMatrixRotationQuaternion(light->GetGameObject()->WorldRotation()),
		XMMatrixTranslation(lightCameraPos.x, lightCameraPos.y, lightCameraPos.z));
	shadowCameraView = DirectX::XMMatrixInverse(&XMMatrixDeterminant(shadowCameraView), shadowCameraView);
	XMMATRIX shadowCameraproj = XMMatrixOrthographicLH(orthoWidth, orthoHeight, nearPlane, farPlane);

// 	XMMATRIX shadowCameraView = XMMatrixMultiply(XMMatrixRotationQuaternion(light->GetGameObject()->WorldRotation()),
// 		XMMatrixTranslation(0.0f, 10.0f, -10.0f));
// 	shadowCameraView = DirectX::XMMatrixInverse(&XMMatrixDeterminant(shadowCameraView), shadowCameraView);
// 	XMMATRIX shadowCameraproj = XMMatrixOrthographicLH(30, 30, 1, 30);

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

	m_ShadowMapPassCBData.EyePosW = nearPlaneCenter;
	m_ShadowMapPassCBData.NearZ = nearPlane;
	m_ShadowMapPassCBData.FarZ = farPlane;

	memcpy(shadowPerPassCB, &m_ShadowMapPassCBData, sizeof(PerPassConstants));
}

void ForwardRenderer::ComputeIrradianceMapAndFilterEnvMap()
{
	PerPassConstants perPassConstants;

	std::array<XMMATRIX, 6> viewMatrices = {
		DirectX::XMMatrixRotationRollPitchYaw(0.0f, -MathHelper::Pi / 2.0f, 0.0f),	/* +X */
		DirectX::XMMatrixRotationRollPitchYaw(0.0f, MathHelper::Pi / 2.0f, 0.0f),	/* -X */
		DirectX::XMMatrixRotationRollPitchYaw(MathHelper::Pi / 2.0f, 0.0f, 0.0f),	/* +Y */
		DirectX::XMMatrixRotationRollPitchYaw(-MathHelper::Pi / 2.0f, 0.0f, 0.0f),	/* -Y */
		DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f),					/* +Z */
		DirectX::XMMatrixRotationRollPitchYaw(0.0f, -MathHelper::Pi, 0.0f)			/* -Z */
	};
	XMMATRIX projMatrix = DirectX::XMMatrixPerspectiveFovLH(MathHelper::Pi / 2.0f, 1, 0.001f, 100);

	perPassConstants.EyePosW = {0.0f, 0.0f, 0.0f};
	perPassConstants.NearZ = 0.001f;
	perPassConstants.FarZ = 100;

	std::shared_ptr<RHI::GpuDynamicBuffer> perPassCB = std::make_shared<RHI::GpuDynamicBuffer>(1, sizeof(PerPassConstants));

	RHI::ShaderVariable* irradiancePerPassVariable = m_PrecomputeIrradianceMapPSO->GetStaticVariableByName(RHI::SHADER_TYPE_VERTEX, "cbPass");
	irradiancePerPassVariable->Set(perPassCB);

	RHI::GraphicsContext& graphicContext = RHI::GraphicsContext::Begin(L"ComputeIrradianceMapAndFilterEnvMap");

	ID3D12DescriptorHeap* cbvsrvuavHeap = RHI::RenderDevice::GetSingleton().GetGPUDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).GetD3D12DescriptorHeap();
	ID3D12DescriptorHeap* samplerHeap = RHI::RenderDevice::GetSingleton().GetGPUDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER).GetD3D12DescriptorHeap();
	graphicContext.SetDescriptorHeap(cbvsrvuavHeap, samplerHeap);

	// ------------------------------Precompute irradiance map--------------------------------------------------------------------
	graphicContext.TransitionResource(*m_IrradianceMap, D3D12_RESOURCE_STATE_RENDER_TARGET);

	graphicContext.SetPipelineState(m_PrecomputeIrradianceMapPSO.get());

	graphicContext.SetVertexBuffer(0, m_CubeMesh->VertexBufferView());
	graphicContext.SetIndexBuffer(m_CubeMesh->IndexBufferView());
	graphicContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT16 mipLevels = m_IrradianceMap->GetResource()->GetDesc().MipLevels;

	for (UINT mip = 0; mip < mipLevels; ++mip)
	{
		// 注意设置Viewport和Scissor
		UINT32 mipRTSize = IrradianceCubeDim / Math::Pow(2, mip);
		CD3DX12_VIEWPORT irradianceViewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(mipRTSize), static_cast<float>(mipRTSize));
		CD3DX12_RECT irradianceScissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(mipRTSize), static_cast<LONG>(mipRTSize));
		graphicContext.SetViewport(irradianceViewport);
		graphicContext.SetScissor(irradianceScissorRect);

		for (UINT face = 0; face < 6; ++face)
		{
			std::shared_ptr<GpuResourceDescriptor> rtv = m_IrradianceMap->CreateRTV(face, mip);
			GpuResourceDescriptor* pRTV = rtv.get();

			graphicContext.ClearColor(*rtv, Colors::Blue);
			graphicContext.SetRenderTargets(1, &pRTV, nullptr);

			// 旋转相机
			XMMATRIX viewProj = XMMatrixMultiply(viewMatrices[face], projMatrix);
			XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(viewMatrices[face]), viewMatrices[face]);
			XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(projMatrix), projMatrix);
			XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

			XMStoreFloat4x4(&perPassConstants.View, XMMatrixTranspose(viewMatrices[face]));
			XMStoreFloat4x4(&perPassConstants.InvView, XMMatrixTranspose(invView));
			XMStoreFloat4x4(&perPassConstants.Proj, XMMatrixTranspose(projMatrix));
			XMStoreFloat4x4(&perPassConstants.InvProj, XMMatrixTranspose(invProj));
			XMStoreFloat4x4(&perPassConstants.ViewProj, XMMatrixTranspose(viewProj));
			XMStoreFloat4x4(&perPassConstants.InvViewProj, XMMatrixTranspose(invViewProj));

			void* pIrradiancePerPassCB = perPassCB->Map(graphicContext, 256);
			memcpy(pIrradiancePerPassCB, &perPassConstants, sizeof(PerPassConstants));

			graphicContext.DrawIndexedInstanced(m_CubeMesh->IndexCount(), 1, 0, 0, 0);
		}
	}

	graphicContext.TransitionResource(*m_IrradianceMap, D3D12_RESOURCE_STATE_GENERIC_READ);
	// ------------------------------Precompute irradiance map--------------------------------------------------------------------

	// ------------------------------Pre filter Env map--------------------------------------------------------------------
	RHI::ShaderVariable* prefilterPerPassVariable = m_PrefilterEnvPSO->GetStaticVariableByName(RHI::SHADER_TYPE_VERTEX, "cbPass");
	prefilterPerPassVariable->Set(perPassCB);

	PrefilterEnvmapConstants prefilterConstants = {};
	std::shared_ptr<RHI::GpuDynamicBuffer> preFilterCB = std::make_shared<RHI::GpuDynamicBuffer>(1, sizeof(PrefilterEnvmapConstants));
	RHI::ShaderVariable* prefilterEnvVariable = m_PrefilterEnvPSO->GetStaticVariableByName(RHI::SHADER_TYPE_PIXEL, "cbPrefilterEnv");
	if(prefilterEnvVariable != nullptr)
		prefilterEnvVariable->Set(preFilterCB);

	graphicContext.TransitionResource(*m_PrefilterEnvMap, D3D12_RESOURCE_STATE_RENDER_TARGET);

	graphicContext.SetPipelineState(m_PrefilterEnvPSO.get());

	mipLevels = m_PrefilterEnvMap->GetResource()->GetDesc().MipLevels;
	for (UINT mip = 0; mip < mipLevels; ++mip)
	{
		for (UINT face = 0; face < 6; ++face)
		{
			UINT32 mipRTSize = PrefilteredEnvMapDim / Math::Pow(2, mip);
			CD3DX12_VIEWPORT prefilterEnvViewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(mipRTSize), static_cast<float>(mipRTSize));
			CD3DX12_RECT prefilterEnvScissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(mipRTSize), static_cast<LONG>(mipRTSize));
			graphicContext.SetViewport(prefilterEnvViewport);
			graphicContext.SetScissor(prefilterEnvScissorRect);

			std::shared_ptr<GpuResourceDescriptor> rtv = m_PrefilterEnvMap->CreateRTV(face, mip);
			GpuResourceDescriptor* pRTV = rtv.get();

			graphicContext.ClearColor(*rtv, Colors::Blue);
			graphicContext.SetRenderTargets(1, &pRTV, nullptr);

			// 旋转相机
			XMMATRIX viewProj = XMMatrixMultiply(viewMatrices[face], projMatrix);
			XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(viewMatrices[face]), viewMatrices[face]);
			XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(projMatrix), projMatrix);
			XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

			XMStoreFloat4x4(&perPassConstants.View, XMMatrixTranspose(viewMatrices[face]));
			XMStoreFloat4x4(&perPassConstants.InvView, XMMatrixTranspose(invView));
			XMStoreFloat4x4(&perPassConstants.Proj, XMMatrixTranspose(projMatrix));
			XMStoreFloat4x4(&perPassConstants.InvProj, XMMatrixTranspose(invProj));
			XMStoreFloat4x4(&perPassConstants.ViewProj, XMMatrixTranspose(viewProj));
			XMStoreFloat4x4(&perPassConstants.InvViewProj, XMMatrixTranspose(invViewProj));

			void* pPrefilterPerPassCB = perPassCB->Map(graphicContext, 256);
			memcpy(pPrefilterPerPassCB, &perPassConstants, sizeof(PerPassConstants));

			prefilterConstants.EnvMapSize = PrefilteredEnvMapDim;
			prefilterConstants.NumSamples = 256;
			prefilterConstants.Roughness = 0.9f;// (float)mip / (float)mipLevels;

			void* pPrefilterCB = preFilterCB->Map(graphicContext, 256);
			memcpy(pPrefilterCB, &prefilterConstants, sizeof(PrefilterEnvmapConstants));

			graphicContext.DrawIndexedInstanced(m_CubeMesh->IndexCount(), 1, 0, 0, 0);
		}
	}

	graphicContext.TransitionResource(*m_PrefilterEnvMap, D3D12_RESOURCE_STATE_GENERIC_READ);


	graphicContext.Finish();
}

void ForwardRenderer::PrecomputeBRDF()
{
	m_FullScreenTriangle = GeometryFactory::CreateFullScreenTriangle();
	std::shared_ptr<RHI::GpuResourceDescriptor> lutRTV = m_BRDF_Lut->CreateRTV();
	RHI::GpuResourceDescriptor* plutRTV = lutRTV.get();

	RHI::GraphicsContext& graphicContext = RHI::GraphicsContext::Begin(L"PreComputeBRDF");

	ID3D12DescriptorHeap* cbvsrvuavHeap = RHI::RenderDevice::GetSingleton().GetGPUDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).GetD3D12DescriptorHeap();
	ID3D12DescriptorHeap* samplerHeap = RHI::RenderDevice::GetSingleton().GetGPUDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER).GetD3D12DescriptorHeap();
	graphicContext.SetDescriptorHeap(cbvsrvuavHeap, samplerHeap);

	graphicContext.TransitionResource(*m_BRDF_Lut, D3D12_RESOURCE_STATE_RENDER_TARGET);

	graphicContext.SetPipelineState(m_PrecomputeBRDFPSO.get());

	graphicContext.ClearColor(*plutRTV, Colors::Blue);
	graphicContext.SetRenderTargets(1, &plutRTV, nullptr);

	graphicContext.SetVertexBuffer(0, m_FullScreenTriangle->VertexBufferView());
	graphicContext.SetIndexBuffer(m_FullScreenTriangle->IndexBufferView());
	graphicContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	CD3DX12_VIEWPORT brdfViewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(BRDF_LUT_Dim), static_cast<float>(BRDF_LUT_Dim));
	CD3DX12_RECT brdfScissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(BRDF_LUT_Dim), static_cast<LONG>(BRDF_LUT_Dim));
	graphicContext.SetViewport(brdfViewport);
	graphicContext.SetScissor(brdfScissorRect);

	graphicContext.DrawIndexedInstanced(m_FullScreenTriangle->IndexCount(), 1, 0, 0, 0);

	graphicContext.TransitionResource(*m_BRDF_Lut, D3D12_RESOURCE_STATE_GENERIC_READ);

	graphicContext.Finish();
}
