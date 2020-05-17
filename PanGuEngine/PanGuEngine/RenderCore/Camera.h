#pragma once
#include "Component.h"
using namespace DirectX;


class Camera : public Component
{
public:
private:
	bool m_IsOrthographic;
	float m_Aspect;
	float m_NearPlane;
	float m_FarPlane;

	// 正交相机
	float m_OrthographicSize;

	// 透视相机
	float m_FieldOfView;

	// Matrix
	XMFLOAT4X4 m_View;
	XMFLOAT4X4 m_Proj;
};

