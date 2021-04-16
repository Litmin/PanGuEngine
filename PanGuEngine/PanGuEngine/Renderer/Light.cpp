#include "pch.h"
#include "Light.h"
#include "GameObject.h"
#include "SceneManager.h"

void Light::OnAddToGameObject()
{
	SceneManager::GetSingleton().AddLight(this);
}

void Light::OnTransformUpdate()
{
	DirectX::XMStoreFloat3(&m_LightDir, m_GameObject->ForwardDir());
}
