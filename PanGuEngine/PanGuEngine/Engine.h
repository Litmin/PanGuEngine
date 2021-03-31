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

struct EngineCreateInfo
{

};

class Engine
{
public:
	Engine();
	virtual ~Engine();
	static Engine* Get() { return m_Engine; }

public:
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

public:
	ID3D12Resource* CurrentBackBuffer() const;
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

	void FlushCommandQueue();

private:
	void InitialDirect3D();
	void InitialMainWindow();
	void CreateCommandObjects();
	void CreateRtvAndDsvDescriptorHeaps();

private:
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


	std::unique_ptr<SceneManager> m_SceneManager;
	std::unique_ptr<ShaderManager> m_ShaderManager;
	std::unique_ptr<ResourceManager> m_ResourceManager;
	std::unique_ptr<GraphicContext> m_GraphicContext;

	//<-------------------------------Command Object------------------------------------------------->
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;
	//<-------------------------------Command Object------------------------------------------------->

	//<--------------------------------Resource Binding------------------------------------------------------>
	UINT m_RtvDescriptorSize;
	UINT m_DsvDescriptorSize = 0;
	UINT m_CbvSrvUavDescriptorSize = 0;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RtvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DsvHeap;

	//<-------------------------------Resource Binding-------------------------------------------->

	//<-------------------------------Sync---------------------------------------------->
	HANDLE m_FenceEvent;
	Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
	UINT64 m_FenceValue;
	//<-------------------------------Sync---------------------------------------------->

	// TODO:实现FrameResource
	std::vector<std::unique_ptr<FrameResource>> m_FrameResources;

	//<--------------------------------Constant Buffer--------------------------------------->
	std::unique_ptr<UploadBuffer<ObjectConstants>> m_ObjCB;
	//<--------------------------------Constant Buffer--------------------------------------->
};