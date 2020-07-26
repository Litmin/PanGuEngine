#include "pch.h"
#include "Material.h"
#include "ShaderManager.h"

Material::Material(Shader* shader) :
	m_Shader(shader)
{
}

Material::~Material()
{
}

void Material::SetDescriptorTable(UINT propertyID, CD3DX12_GPU_DESCRIPTOR_HANDLE descriptorTable)
{
	m_DescriptorTables[propertyID] = descriptorTable;
}

void Material::SetDescriptorTable(const std::string& property, CD3DX12_GPU_DESCRIPTOR_HANDLE descriptorTable)
{
	m_DescriptorTables[ShaderManager::GetSingleton().PropertyToID(property)] = descriptorTable;
}

void Material::SetConstantBuffer(UINT propertyID, D3D12_GPU_VIRTUAL_ADDRESS address)
{
	m_ConstantBuffers[propertyID] = address;
}

void Material::SetConstantBuffer(const std::string& property, D3D12_GPU_VIRTUAL_ADDRESS address)
{
	m_ConstantBuffers[ShaderManager::GetSingleton().PropertyToID(property)] = address;
}

void Material::BindParameters(ID3D12GraphicsCommandList* commandList)
{
	for (auto& [propertyID, descriptorTable] : m_DescriptorTables)
	{
		m_Shader->SetDescriptorTable(commandList, propertyID, descriptorTable);
	}
	for (auto& [propertyID, cbv] : m_ConstantBuffers)
	{
		m_Shader->SetRootConstantBufferView(commandList, propertyID, cbv);
	}
}
