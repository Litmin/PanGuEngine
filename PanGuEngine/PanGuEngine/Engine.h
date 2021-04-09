#pragma once
#include "GameTimer.h"
#include "Renderer/MeshRenderer.h"
#include "Renderer/Light.h"
#include "Renderer/Material.h"
#include "Renderer/SceneManager.h"
#include "Resource/ResourceManager.h"
#include "D3D12RHI/RenderDevice.h"
#include "D3D12RHI/CommandListManager.h"
#include "D3D12RHI/CommandContext.h"
#include "D3D12RHI/SwapChain.h"
#include "D3D12RHI/GpuBuffer.h"
#include "D3D12RHI/PipelineState.h"
#include "D3D12RHI/ShaderResourceBinding.h"
#include "D3D12RHI/Shader.h"
#include "Renderer/VertexFactory.h"
#include "Renderer/ForwardRenderer.h"

struct EngineCreateInfo
{
	UINT Width;
	UINT Height;
};

class Engine
{
public:
	Engine(HINSTANCE hInstance);
	virtual ~Engine();
	static Engine* Get() { return m_Engine; }

	template<typename TSetup>
	int RunN(const EngineCreateInfo& engineCI, TSetup setup);

	bool IsInitialized() { return m_Initialized; }
	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	void Initialize(const EngineCreateInfo& engineCI);
	void Tick();
	void Update(float deltaTime);
	void Render();
	void Destroy();

	void Pause();
	void Resume();
	void OnResize();
	void SetScreenSize(UINT width, UINT height);
	void CalculateFrameStats();

	void InitialMainWindow();

	static Engine* m_Engine;

	bool m_Initialized = false;

	//<--------------------------------Windows------------------------------------------>
	HINSTANCE m_AppInst;
	HWND m_MainWnd;
	//<--------------------------------Windows------------------------------------------>

	GameTimer m_Timer;
	bool m_Paused = false;

	UINT m_Width;
	UINT m_Height;
	float m_Aspect;
	bool m_resizing = false;
	bool m_minimized = false;
	bool m_maximized = false;


	Microsoft::WRL::ComPtr< IDXGIFactory4> m_DXGIFactory;

	std::unique_ptr<RHI::RenderDevice> m_RenderDevice;
	std::unique_ptr<RHI::CommandListManager> m_CommandListManager; // TODO: Move To RenderDevice
	std::unique_ptr<RHI::ContextManager> m_CommandContextManager;	// TODO: Move To RenderDevice

	std::unique_ptr<RHI::SwapChain> m_SwapChain;


	std::unique_ptr<SceneManager> m_SceneManager;
	std::unique_ptr<VertexFactory> m_vertexFactory;

	std::unique_ptr<ForwardRenderer> m_ForwardRenderer;
};

template<typename TSetup>
int Engine::RunN(const EngineCreateInfo& engineCI, TSetup setup)
{
	Initialize(engineCI);
	setup();


	m_Timer.Reset();
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		m_Timer.Tick();
		Tick();
	}

	Destroy();
	return (int)msg.wParam;
}
