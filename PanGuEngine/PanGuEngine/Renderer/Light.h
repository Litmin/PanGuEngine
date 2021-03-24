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

	DirectX::XMFLOAT3 direction;
	float intensity;
	DirectX::XMFLOAT3 color;
};

