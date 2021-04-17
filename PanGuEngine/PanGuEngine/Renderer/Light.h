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

	void SetLightColor(DirectX::XMFLOAT3 color);
	void SetLightIntensity(float intensity);

	void UpdateLightCB(void* lightCB);

private:
	LightConstants m_LightConstants;
};

