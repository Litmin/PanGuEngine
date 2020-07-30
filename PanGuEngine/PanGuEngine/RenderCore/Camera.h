#pragma once
#include "MovableObject.h"

extern const int gNumFrameResources;
class Camera : public MovableObject
{
public:
	Camera();
	virtual ~Camera();

	void SetProjection(float aspect, float nearPlane, float farPlane, float fieldOfView);

	void UpdateCameraCBs();

protected:
	float m_Aspect;
	float m_NearPlane;
	float m_FarPlane;

	// TODO:正交相机
	// 透视相机
	float m_FieldOfView;

	// Matrix
	DirectX::XMFLOAT4X4 m_View;
	DirectX::XMFLOAT4X4 m_Proj;


	int m_NumFramesDirty = gNumFrameResources;
};

