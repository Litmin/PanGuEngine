#include "pch.h"
#include "Shader.h"
#include <array>
#include "..\D3D12RHI\Shader.h"

using namespace std;
using Microsoft::WRL::ComPtr;

Shader::Shader()
{
	// 绑定通用参数
	// Per Object Buffer
	ShaderParameter perObject("cbPerObject", ShaderParamType::CBVDescriptorHeap, 1, 0, 0);
	m_ParamMap[ShaderManager::GetSingleton().PropertyToID("cbPerObject")] = m_Params.size();
	m_Params.push_back(perObject);

	// Per Pass Buffer
	ShaderParameter perPass("cbPass", ShaderParamType::CBVDescriptorHeap, 1, 1, 0);
	m_ParamMap[ShaderManager::GetSingleton().PropertyToID("cbPass")] = m_Params.size();
	m_Params.push_back(perPass);

	// 光照参数


	// 每个Shader可以override这些渲染状态
	m_RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	m_BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	m_DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
}

void Shader::Initialize(ID3D12Device* device)
{
	// 编译Shader
	m_VS = d3dUtil::CompileShader(m_FilePath, nullptr, m_VSEntry, "vs_5_0");
	m_PS = d3dUtil::CompileShader(m_FilePath, nullptr, m_PSEntry, "ps_5_0");

	// 根据参数创建根签名
	vector<CD3DX12_ROOT_PARAMETER> rootParameters;
	vector<CD3DX12_DESCRIPTOR_RANGE> texTables;
	vector<CD3DX12_DESCRIPTOR_RANGE> constantTables;

	// TODO:用更好的方式初始化根签名参数
	constantTables.reserve(m_Params.size());

	for (auto& shaderParameter : m_Params)
	{
		CD3DX12_ROOT_PARAMETER rootParameter;

		switch (shaderParameter.type)
		{
		case ShaderParamType::ConstantBuffer:
			rootParameter.InitAsConstantBufferView(shaderParameter.baseRegister, shaderParameter.registerSpace);
			break;
		case ShaderParamType::CBVDescriptorHeap:
			CD3DX12_DESCRIPTOR_RANGE constantTable;
			constantTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, shaderParameter.descriptorNums, shaderParameter.baseRegister, shaderParameter.registerSpace);
			constantTables.push_back(constantTable);
			rootParameter.InitAsDescriptorTable(1, &constantTables.back());

			break;
		case ShaderParamType::SRVDescriptorHeap:
			CD3DX12_DESCRIPTOR_RANGE texTable;
			texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, shaderParameter.descriptorNums, shaderParameter.baseRegister, shaderParameter.registerSpace);
			texTables.push_back(texTable);
			rootParameter.InitAsDescriptorTable(1, &texTables.back());
			break;
		}

		rootParameters.push_back(rootParameter);
	}

	auto staticSamplers = GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(rootParameters.size(), rootParameters.data(),
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(device->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(m_RootSignature.GetAddressOf())));
}

void Shader::BindRootSignature(ID3D12GraphicsCommandList* commandList)
{
	commandList->SetGraphicsRootSignature(m_RootSignature.Get());
}

void Shader::SetDescriptorTable(ID3D12GraphicsCommandList* commandList, UINT paramID, CD3DX12_GPU_DESCRIPTOR_HANDLE handle)
{
	auto&& ite = m_ParamMap.find(paramID);
	if (ite == m_ParamMap.end())
		return;

	UINT rootParamIndex = ite->second;
	ShaderParameter& parameter = m_Params[rootParamIndex];

	commandList->SetGraphicsRootDescriptorTable(rootParamIndex, handle);
}

void Shader::SetRootConstantBufferView(ID3D12GraphicsCommandList* commandList, UINT paramID, D3D12_GPU_VIRTUAL_ADDRESS address)
{
	auto&& ite = m_ParamMap.find(paramID);
	if (ite == m_ParamMap.end())
		return;

	UINT rootParamIndex = ite->second;
	ShaderParameter& parameter = m_Params[rootParamIndex];

	commandList->SetGraphicsRootConstantBufferView(rootParamIndex, address);
}

void Shader::SetPSODesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* psoDesc)
{
	psoDesc->VS =
	{
		reinterpret_cast<BYTE*>(m_VS->GetBufferPointer()),
		m_VS->GetBufferSize()
	};
	psoDesc->PS =
	{
		reinterpret_cast<BYTE*>(m_PS->GetBufferPointer()),
		m_PS->GetBufferSize()
	};
	psoDesc->pRootSignature = m_RootSignature.Get();
	psoDesc->RasterizerState = m_RasterizerState;
	psoDesc->DepthStencilState = m_DepthStencilState;
	psoDesc->BlendState = m_BlendState;
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> Shader::GetStaticSamplers()
{
	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.  

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp };
}
