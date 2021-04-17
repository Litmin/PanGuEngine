#include "pch.h"
#include "Light.h"
#include "GameObject.h"
#include "SceneManager.h"

void Light::OnAddToGameObject()
{
	SceneManager::GetSingleton().AddLight(this);
	// 初始化光源方向
	DirectX::XMStoreFloat3(&m_LightConstants.LightDir, m_GameObject->ForwardDir());
}

void Light::OnTransformUpdate()
{
	DirectX::XMStoreFloat3(&m_LightConstants.LightDir, m_GameObject->ForwardDir());
}

void Light::SetLightColor(XMFLOAT3 color)
{
	m_LightConstants.LightColor = color;
}

void Light::SetLightIntensity(float intensity)
{
	m_LightConstants.LightIntensity = intensity;
}

void Light::UpdateLightCB(void* lightCB)
{
	//m_LightConstants.LightDir = DirectX::XMFLOAT3(0.0f, -1.0f, 1.0f);
	memcpy(lightCB, &m_LightConstants, sizeof(LightConstants));
}
