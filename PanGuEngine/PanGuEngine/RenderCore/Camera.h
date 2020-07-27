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

	// �������
	float m_OrthographicSize;

	// ͸�����
	float m_FieldOfView;

	// Matrix
	DirectX::XMFLOAT4X4 m_View;
	DirectX::XMFLOAT4X4 m_Proj;
};

