#pragma once
#include "GameTimer.h"
#include "Renderer/FrameResource.h"
#include "Renderer/MeshRenderer.h"
#include "Renderer/Light.h"
#include "Renderer/Material.h"
#include "Renderer/UploadBuffer.h"
#include "Renderer/SceneManager.h"
#include "Renderer/ShaderManager.h"
#include "Resource/ResourceManager.h"
#include "Renderer/GraphicContext.h"
#include "D3D12RHI/RenderDevice.h"
#include "D3D12RHI/CommandListManager.h"
#include "D3D12RHI/CommandContext.h"
#include "D3D12RHI/SwapChain.h"


class Engine
{
public:
	Engine();
	virtual ~Engine();
	static Engine* Get() { return m_Engine; }

	void Initialize(UINT width, UINT height, HINSTANCE hInstance);
	bool IsInitialized() { return m_Initialized; }
	int Run();
	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
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
	//wstring m_Title;

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

	CD3DX12_VIEWPORT m_Viewport;
	CD3DX12_RECT m_ScissorRect;

	Microsoft::WRL::ComPtr< IDXGIFactory4> m_DXGIFactory;

	std::unique_ptr<RHI::RenderDevice> m_RenderDevice;
	std::unique_ptr<RHI::CommandListManager> m_CommandListManager; // TODO: Move To RenderDevice
	std::unique_ptr<RHI::ContextManager> m_CommandContextManager;	// TODO: Move To RenderDevice

	std::unique_ptr<RHI::SwapChain> m_SwapChain;


	std::unique_ptr<SceneManager> m_SceneManager;
	std::unique_ptr<ResourceManager> m_ResourceManager;
};