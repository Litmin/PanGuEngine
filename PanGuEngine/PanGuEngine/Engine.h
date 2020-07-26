#pragma once
#include "GameTimer.h"
#include "RenderCore/FrameResource.h"
#include "RenderCore/MeshRenderer.h"
#include "RenderCore/Light.h"
#include "RenderCore/Material.h"
#include "RenderCore/UploadBuffer.h"
#include "RenderCore/SceneManager.h"
#include "RenderCore/ShaderManager.h"
#include "Resource/ResourceManager.h"
#include "RenderCore/GraphicContext.h"

class Engine
{
public:
	Engine();
	virtual ~Engine();

public:
	void Initialize(UINT width, UINT height, HWND hwnd);
	bool IsInitialized() { return m_Initialized; }
	void Tick();
	void Destroy();

public:
	void Pause();
	void Resume();
	void OnResize();
	void SetScreenSize(UINT width, UINT height);

public:
	PBRMaterial* CreateMaterial(float albedo, float metallic, float smoothness);
	MeshRenderer CreateRenderer(DirectX::XMFLOAT4X4 worldTransform, Mesh* mesh, PBRMaterial* material);

	void UpdateConstantBuffer();

private:
	void Update();
	void Render();


private:
	void InitialDirect3D(HWND hwnd);
	void CreateCommandObjects();
	void CreateSwapChain();
	void CreateRtvAndDsvDescriptorHeaps();

	void BuildFrameResources();

	void FlushCommandQueue();

private:
	static const int SwapChainBufferCount = 2;

	bool m_Initialized = false;

	HWND m_MainWnd;
	GameTimer m_Timer;

	UINT m_Width;
	UINT m_Height;
	float m_Aspect;

	CD3DX12_VIEWPORT m_Viewport;
	CD3DX12_RECT m_ScissorRect;

	Microsoft::WRL::ComPtr< IDXGIFactory4> m_DXGIFactory;
	Microsoft::WRL::ComPtr<ID3D12Device> m_Device;

	Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;

	//<-------------------------------Command Object------------------------------------------------->
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;
	//<-------------------------------Command Object------------------------------------------------->

	//<-------------------------------Render State------------------------------------------->
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineState;
	ComPtr<ID3DBlob> mvsByteCode = nullptr;
	ComPtr<ID3DBlob> mpsByteCode = nullptr;
	//<-------------------------------Render State------------------------------------------->

	//<--------------------------------Resource Binding------------------------------------------------------>
	UINT m_RtvDescriptorSize;
	UINT m_DsvDescriptorSize = 0;
	UINT m_CbvSrvUavDescriptorSize = 0;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RtvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DsvHeap;

	int m_CurrBackBuffer = 0;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_SwapChainBuffer[SwapChainBufferCount];
	Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthStencilBuffer;
	//<-------------------------------Resource Binding-------------------------------------------->

	//<-------------------------------Sync---------------------------------------------->
	HANDLE m_FenceEvent;
	Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
	UINT64 m_FenceValue;
	//<-------------------------------Sync---------------------------------------------->

	// TODO:实现FrameResource
	std::vector<std::unique_ptr<FrameResource>> m_FrameResources;

	// TODO:实现材质系统
	std::vector<MeshRenderer*> m_MeshRendererUpdateConstantQueue;

	std::vector<MeshRenderer*> m_AllRenderer;
	// TODO:实现光照
	std::vector<Light*> m_Lights;

	//<--------------------------------Constant Buffer--------------------------------------->
	std::unique_ptr<UploadBuffer<ObjectConstants>> m_ObjCB;
	//<--------------------------------Constant Buffer--------------------------------------->

	std::unique_ptr<SceneManager> m_SceneManager;
	std::unique_ptr<ShaderManager> m_ShaderManager;
	std::unique_ptr<ResourceManager> m_ResourceManager;

	std::unique_ptr<GraphicContext> m_GraphicContext;
};