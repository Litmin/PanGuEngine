#pragma once
#include "Shader.h"

struct PipelineStateDesc
{
	Shader* shaderPtr;
	UINT meshLayoutIndex;
	D3D12_PRIMITIVE_TOPOLOGY_TYPE topology;

	size_t hash;
	bool operator==(const PipelineStateDesc& other)const;
	PipelineStateDesc() :
		topology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
	{

	}
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

namespace std
{
	template <>
	struct hash<PipelineStateDesc>
	{
		size_t operator()(const PipelineStateDesc& key) const
		{
			return key.hash;
		}
	};
	template <>
	struct hash<std::pair<UINT, PipelineStateDesc>>
	{
		size_t operator()(const std::pair<UINT, PipelineStateDesc>& key) const
		{
			return key.first ^ key.second.hash;
		}
	};
}


