#pragma once
#include "Component.h"

class Light : public Component
{
public:
	enum class LightTypes
	{
		Direction,
		Point,
		SpotLight,

		NUM_LIGHT_TYPEs
	};

	virtual void OnAddToGameObject() override;
	virtual void OnTransformUpdate() override;

private:
	DirectX::XMFLOAT3 m_LightDir = { 0.0f, 0.0f, 0.0f };
	float m_LightIntensity = 1.0f;
	DirectX::XMFLOAT3 m_LightColor = { 0.0f, 0.0f, 0.0f };
};

