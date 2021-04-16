#pragma once


namespace RHI
{
	class GpuTexture2D;
	class GpuDynamicBuffer;
	class Shader;
	class PipelineState;
	class SwapChain;
}

class ForwardRenderer
{
public:
	void Initialize();
	void Render(RHI::SwapChain& swapChain);

private:
	std::shared_ptr<RHI::GpuDynamicBuffer> m_PerDrawCB = nullptr;
	std::shared_ptr<RHI::GpuDynamicBuffer> m_PerPassCB = nullptr;
	std::shared_ptr<RHI::GpuDynamicBuffer> m_LightCB = nullptr;

	std::shared_ptr<RHI::Shader> m_StandardVS = nullptr;
	std::shared_ptr<RHI::Shader> m_StandardPS = nullptr;
	std::unique_ptr<RHI::PipelineState> m_PSO = nullptr;
};

