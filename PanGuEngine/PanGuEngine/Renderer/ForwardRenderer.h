#pragma once


namespace RHI
{
	class GpuTexture2D;
	class GpuDynamicBuffer;
	class GpuRenderTextureDepth;
	class GpuResourceDescriptor;
	class Shader;
	class PipelineState;
	class SwapChain;
	class GpuCubemap;
}

class Light;
class Camera;

class ForwardRenderer
{
public:
	void Initialize();
	void Render(RHI::SwapChain& swapChain);

private:
	void UpdateShadowPerPassCB(Light* light, Camera* sceneCamera, void* shadowPerPassCB);

	// Main Pass
	std::shared_ptr<RHI::GpuDynamicBuffer> m_PerDrawCB = nullptr;
	std::shared_ptr<RHI::GpuDynamicBuffer> m_PerPassCB = nullptr;
	std::shared_ptr<RHI::GpuDynamicBuffer> m_LightCB = nullptr;

	std::shared_ptr<RHI::Shader> m_StandardVS = nullptr;
	std::shared_ptr<RHI::Shader> m_StandardPS = nullptr;
	std::unique_ptr<RHI::PipelineState> m_MainPassPSO = nullptr;

	// ShadowMap Pass
	std::shared_ptr<RHI::GpuDynamicBuffer> m_ShadowMapPerDrawCB = nullptr;
	std::shared_ptr<RHI::GpuDynamicBuffer> m_ShadowMapPerPassCB = nullptr;

	std::shared_ptr<RHI::Shader> m_ShadowMapVS = nullptr;
	std::shared_ptr<RHI::Shader> m_ShadowMapPS = nullptr;
	std::unique_ptr<RHI::PipelineState> m_ShadowMapPSO = nullptr;

	std::shared_ptr<RHI::GpuRenderTextureDepth> m_ShadowMap = nullptr;
	std::shared_ptr<RHI::GpuResourceDescriptor> m_ShadowMapDSV = nullptr;
	std::shared_ptr<RHI::GpuResourceDescriptor> m_ShadowMapSRV = nullptr;

	PerPassConstants m_ShadowMapPassCBData;

	const float m_ShadowMapSize = 4096.0f;
	CD3DX12_VIEWPORT m_ShadowMapViewport;
	CD3DX12_RECT m_ShadowMapScissorRect;

	// Skybox
	std::shared_ptr<RHI::GpuResourceDescriptor> m_SkyboxSRV = nullptr;
};

