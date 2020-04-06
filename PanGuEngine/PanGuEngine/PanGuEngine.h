#pragma once

#include "Vertex.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

class PanGuEngine
{
public:
	PanGuEngine();
	virtual ~PanGuEngine();

public:
	void Initialize(UINT width, UINT height, HWND hwnd);
	void Tick();
	void Destroy();

public:
	void Pause();
	void Resume();
	void SetScreenSize(UINT width, UINT height);
	void OnResize();

private:
	void Update();
	void Render();

private:
	void GetHardwareAdapter(_In_ IDXGIFactory2* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter);


private:
	static const UINT FrameCount = 2;

	UINT m_width;
	UINT m_height;
	float m_aspect;

	// Pipeline objects.
	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_RECT m_scissorRect;
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	UINT m_rtvDescriptorSize;

	// Vertex Buffer.
	ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	// Synchronization objects.
	UINT m_frameIndex;
	HANDLE m_fenceEvent;
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue;

	void LoadPipeline(HWND hwnd);
	void LoadAssets();
	void PopulateCommandList();
	void WaitForPreviousFrame();
};