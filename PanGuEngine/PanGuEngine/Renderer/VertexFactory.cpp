#include "pch.h"
#include "VertexFactory.h"

using namespace std;

UINT VertexFactory::GetInputLayoutIndex(
	bool hasColor,
	bool hasNormal,
	bool hasTangent,
	bool hasuv0,
	bool hasuv1,
	bool hasuv2,
	bool hasuv3)
{
	USHORT mask = 0;
	if (hasColor)
		mask |= (1 << 0);
	if (hasNormal)
		mask |= (1 << 1);
	if (hasTangent)
		mask |= (1 << 2);
	if (hasuv0)
		mask |= (1 << 3);
	if (hasuv1)
		mask |= (1 << 4);
	if (hasuv2)
		mask |= (1 << 5);
	if (hasuv3)
		mask |= (1 << 6);

	// TODO:Why&&
	auto&& ite = m_InputLayoutMap.find(mask);
	if (ite == m_InputLayoutMap.end())
	{
		unique_ptr<vector<D3D12_INPUT_ELEMENT_DESC>> desc = make_unique<vector<D3D12_INPUT_ELEMENT_DESC>>();
		GenerateInputElementDesc(*desc.get(), hasColor, hasNormal, hasTangent, hasuv0, hasuv1, hasuv2, hasuv3);

		UINT index = m_InputLayouts.size();
		m_InputLayoutMap[mask] = index;

		m_InputLayouts.push_back(move(desc));
		return index;
	}

	return ite->second;
}

std::vector<D3D12_INPUT_ELEMENT_DESC>* VertexFactory::GetInputElementDesc(UINT index)
{
	return m_InputLayouts[index].get();
}

void VertexFactory::GenerateInputElementDesc(
	std::vector<D3D12_INPUT_ELEMENT_DESC>& desc,
	bool hasColor,
	bool hasNormal,
	bool hasTangent,
	bool hasuv0,
	bool hasuv1,
	bool hasuv2,
	bool hasuv3)
{
	desc.reserve(8);
	desc.push_back(
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA , 0 }
	);
	UINT offset = 12;

	if (hasColor)
	{
		desc.push_back(
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		);
		offset += 16;
	}
	else
	{
		desc.push_back(
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		);
	}
	if (hasNormal)
	{
		desc.push_back(
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		);
		offset += 12;
	}
	else
	{
		desc.push_back(
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		);
	}
	if (hasTangent)
	{
		desc.push_back(
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		);
		offset += 16;
	}
	else
	{
		desc.push_back(
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		);
	}
	if (hasuv0)
	{
		desc.push_back(
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		);
		offset += 8;
	}
	else
	{
		desc.push_back(
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		);
	}
	if (hasuv1)
	{
		desc.push_back(
			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, offset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		);
		offset += 8;
	}
	else
	{
		desc.push_back(
			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		);
	}
	if (hasuv2)
	{
		desc.push_back(
			{ "TEXCOORD", 2, DXGI_FORMAT_R32G32_FLOAT, 0, offset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		);
		offset += 8;
	}
	else
	{
		desc.push_back(
			{ "TEXCOORD", 2, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		);
	}
	if (hasuv3)
	{
		desc.push_back(
			{ "TEXCOORD", 3, DXGI_FORMAT_R32G32_FLOAT, 0, offset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		);
		offset += 8;
	}
	else
	{
		desc.push_back(
			{ "TEXCOORD", 3, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		);
	}
}