#include "pch.h"
#include "RootSignature.h"

namespace RHI
{
	void RootSignature::RootParamsManager::AddRootView(D3D12_ROOT_PARAMETER_TYPE ParameterType, 
													   UINT32 RootIndex, 
													   UINT Register, 
													   D3D12_SHADER_VISIBILITY Visibility, 
													   SHADER_RESOURCE_VARIABLE_TYPE VarType)
	{
		m_RootViews.emplace_back(ParameterType, RootIndex, Register, 0u/*Register Space*/, Visibility, VarType);
	}

	void RootSignature::RootParamsManager::AddRootTable(UINT32 RootIndex, 
														D3D12_SHADER_VISIBILITY Visibility, 
														SHADER_RESOURCE_VARIABLE_TYPE VarType, 
														UINT32 NumRangesInNewTable)
	{
		m_RootTables.emplace_back(D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, RootIndex, NumRangesInNewTable, )
	}

	void RootSignature::RootParamsManager::AddDescriptorRanges(UINT32 RootTableInd, UINT32 NumExtraRanges)
	{

	}

	bool RootSignature::RootParamsManager::operator==(const RootParamsManager& RootParams) const
	{
		return false;
	}

	size_t RootSignature::RootParamsManager::GetHash() const
	{
		return size_t();
	}
}