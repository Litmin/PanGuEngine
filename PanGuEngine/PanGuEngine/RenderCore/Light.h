#pragma once
#include "MovableObject.h"

class Light : public MovableObject
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

