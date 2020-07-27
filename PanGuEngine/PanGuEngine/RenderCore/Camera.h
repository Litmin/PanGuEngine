#pragma once
#include "MovableObject.h"

class Camera : public MovableObject
{
public:
	Camera();
	virtual ~Camera();

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
	DirectX::XMFLOAT4X4 m_View;
	DirectX::XMFLOAT4X4 m_Proj;
};

