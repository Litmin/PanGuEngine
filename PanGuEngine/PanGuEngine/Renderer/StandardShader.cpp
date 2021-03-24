#include "pch.h"
#include "StandardShader.h"

StandardShader::StandardShader()
{
	m_FilePath = L"Shaders\\Standard.hlsl";
	m_VSEntry = "VS";
	m_PSEntry = "PS";


	// 变化频率高的参数放在前面

	//m_ParamMap.reserve(m_ParamMap.size() + 4);
	//m_Params.reserve(m_ParamMap.size() + 4);

	//// Albedo
	//ShaderParameter albedo("_Albedo", ShaderParamType::SRVDescriptorHeap, 1, 0, 0);
	//m_ParamMap[ShaderManager::GetSingleton().PropertyToID("_Albedo")] = m_Params.size();
	//m_Params.push_back(albedo);

	//// Metallic
	//ShaderParameter metallic("_Metallic", ShaderParamType::ConstantBuffer, 1, 0, 0);
	//m_ParamMap[ShaderManager::GetSingleton().PropertyToID("_Metallic")] = m_Params.size();
	//m_Params.push_back(metallic);

	//// Smoothness
	//ShaderParameter smoothness("_Smoothness", ShaderParamType::ConstantBuffer, 1, 0, 0);
	//m_ParamMap[ShaderManager::GetSingleton().PropertyToID("_Smoothness")] = m_Params.size();
	//m_Params.push_back(smoothness);

	//// Normal
	//ShaderParameter normal("_Normal", ShaderParamType::ConstantBuffer, 1, 0, 0);
	//m_ParamMap[ShaderManager::GetSingleton().PropertyToID("_Normal")] = m_Params.size();
	//m_Params.push_back(normal);
}

