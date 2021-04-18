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
}

class Light;

class ForwardRenderer
{
public:
	void Initialize();
	void Render(RHI::SwapChain& swapChain);

private:
	void UpdateShadowPerPassCB(Light* light, void* shadowPerPassCB);

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

	// TODO:根据相机的视锥体计算光源相机的位置和视锥体大小， 暂时固定光源相机的位置和视锥体大小
	DirectX::XMFLOAT3 m_ShadowCameraPos = { 0.0f, 100.0f, -100.0f };
	float m_ShadowCameraWidth = 30.0f;
	float m_ShadowCameraHeight = 30.0f;
	float m_ShadowCameraNear = 1.0f;
	float m_ShadowCameraFar = 500.0f;
	PerPassConstants m_ShadowMapPassCBData;

	CD3DX12_VIEWPORT m_ShadowMapViewport;
	CD3DX12_RECT m_ShadowMapScissorRect;
};

