#include "pch.h"
#include "ShaderManager.h"

ShaderManager::ShaderManager() :
	m_CurID(0)
{
	m_PropertyID.reserve(64);

	m_PerObjectBufferID = PropertyToID("PerObjectBuffer");
	m_PerCameraBufferID = PropertyToID("PerCameraBuffer");
	m_PerMaterialBufferID = PropertyToID("PerMaterialBuffer");
}

ShaderManager::~ShaderManager()
{
}

UINT ShaderManager::PropertyToID(std::string property)
{
	auto&& ite = m_PropertyID.find(property);
	if (ite == m_PropertyID.end())
	{
		unsigned int value = m_CurID;
		m_PropertyID[property] = m_CurID;
		++m_CurID;
		return value;
	}
		
	return ite->second;
}
