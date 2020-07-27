#pragma once
#include "RendererStateDesc.h"
#include <unordered_map>

class GraphicContext : public Singleton<GraphicContext>
{
public:
	GraphicContext();
	GraphicContext(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
	~GraphicContext();

	ID3D12Device* Device() { return m_Device; }
	ID3D12GraphicsCommandList* CommandList() { return m_CommandList; }

private:
	ID3D12Device* m_Device;
	ID3D12GraphicsCommandList* m_CommandList;

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
	ID3D12PipelineState* GetPSO(RendererStateDesc& rendererStateDesc, RTStateDesc& rtStateDesc);

private:
	std::unordered_map<std::pair<RTStateDesc, RendererStateDesc>, Microsoft::WRL::ComPtr<ID3D12PipelineState>> m_PSOs;

//***********************PSO*********************************
};

