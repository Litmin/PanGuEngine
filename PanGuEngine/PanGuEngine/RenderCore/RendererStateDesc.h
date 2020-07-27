#pragma once
#include "Shader.h"

struct RendererStateDesc
{
	Shader* shaderPtr;
	UINT meshLayoutIndex;
	D3D12_PRIMITIVE_TOPOLOGY_TYPE topology;

	//RendererStateDesc() :
	//	topology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE){}
	bool operator==(const RendererStateDesc& other)const;

	size_t hash;
	void GenerateHash()
	{
		size_t value = meshLayoutIndex;
		value <<= 4;
		value &= topology;
		value <<= 4;
		value ^= reinterpret_cast<size_t>(shaderPtr);
		std::hash<size_t> h;
		hash = h(value);
	}
};

struct RTStateDesc
{
	DXGI_FORMAT depthFormat;
	UINT rtCount;
	DXGI_FORMAT rtFormat[8];

	bool operator==(const RTStateDesc& other)const;

	size_t hash;
	void GenerateHash()
	{
		size_t value = depthFormat;
		for (UINT i = 0; i < rtCount; ++i)
		{
			value ^= rtFormat[i];
		}
		std::hash<size_t> h;
		hash = h(value);
	}
};

namespace std
{
	template <>
	struct hash<RendererStateDesc>
	{
		size_t operator()(const RendererStateDesc& key) const
		{
			return key.hash;
		}
	};
	template <>
	struct hash<std::pair<RTStateDesc, RendererStateDesc>>
	{
		size_t operator()(const std::pair<RTStateDesc, RendererStateDesc>& key) const
		{
			return key.first.hash ^ key.second.hash;
		}
	};
}


