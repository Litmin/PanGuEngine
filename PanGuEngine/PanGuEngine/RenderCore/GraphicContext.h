#pragma once
#include "RendererStateDesc.h"
#include "FrameResource.h"
#include <unordered_map>


class GraphicContext : public Singleton<GraphicContext>
{
public:
	GraphicContext(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* commandList,
		ID3D12CommandQueue* commandQueue,
		ID3D12Fence* fence);
	~GraphicContext();

	ID3D12Device* Device() { return m_Device; }
	ID3D12GraphicsCommandList* CommandList() { return m_CommandList; }

	void Update();

private:
	ID3D12Device* m_Device;
	ID3D12GraphicsCommandList* m_CommandList;
	ID3D12CommandQueue* m_CommandQueue;
	ID3D12Fence* m_Fence;

//*******************InputLayout*****************************
public:
	// 每种输入顶点的布局对应唯一的索引
	UINT GetInputLayoutIndex(
		bool hasColor,
		bool hasNormal,
		bool hasTangent,
		bool hasuv0,
		bool hasuv1,
		bool hasuv2,
		bool hasuv3);
	std::vector<D3D12_INPUT_ELEMENT_DESC>* GetInputElementDesc(UINT index);
private:
	std::unordered_map<USHORT, UINT> m_InputLayoutMap;
	std::vector<std::unique_ptr<std::vector<D3D12_INPUT_ELEMENT_DESC>>> m_InputLayouts;
	void GenerateInputElementDesc(
		std::vector<D3D12_INPUT_ELEMENT_DESC>& desc,
		bool hasColor,
		bool hasNormal,
		bool hasTangent,
		bool hasuv0,
		bool hasuv1,
		bool hasuv2,
		bool hasuv3);
//*******************InputLayout*****************************

//***********************PSO*********************************
public:
	ID3D12PipelineState* GetPSO(const RendererStateDesc& rendererStateDesc, RTStateDesc& rtStateDesc);

private:
	std::unordered_map<std::pair<RTStateDesc, RendererStateDesc>, Microsoft::WRL::ComPtr<ID3D12PipelineState>> m_PSOs;

//***********************PSO*********************************

//*******************FrameResource***************************
public:
	void BuildFrameResource();
	FrameResource* GetCurrFrameResource() { return m_CurrFrameResource; }

private:
	int m_CurrFrameResourceIndex = 0;
	FrameResource* m_CurrFrameResource = nullptr;
	std::vector<std::unique_ptr<FrameResource>> m_FrameResources;
//*******************FrameResource***************************
};

