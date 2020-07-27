#pragma once
#include "Shader.h"
#include <unordered_map>

//**********************************************
// ����һ�����������:
//		ʹ�õ�Shader
//		Shader����ֵ
//**********************************************
class Material
{
public:
	Material(Shader* shader);
	~Material();

	void SetDescriptorTable(UINT propertyID, CD3DX12_GPU_DESCRIPTOR_HANDLE descriptorTable);
	void SetDescriptorTable(const std::string& property, CD3DX12_GPU_DESCRIPTOR_HANDLE descriptorTable);
	void SetConstantBuffer(UINT propertyID, D3D12_GPU_VIRTUAL_ADDRESS address);
	void SetConstantBuffer(const std::string& property, D3D12_GPU_VIRTUAL_ADDRESS address);
	void BindParameters(ID3D12GraphicsCommandList* commandList);
	Shader* GetShader() { return m_Shader; }
private:
	Shader* m_Shader;
	std::unordered_map<UINT, CD3DX12_GPU_DESCRIPTOR_HANDLE> m_DescriptorTables;
	std::unordered_map<UINT, D3D12_GPU_VIRTUAL_ADDRESS> m_ConstantBuffers;
};
