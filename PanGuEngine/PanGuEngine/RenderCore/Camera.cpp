#include "pch.h"
#include "Camera.h"

using namespace DirectX;

Camera::Camera()
{
	SetProjection(1.0f, 1.0f, 1000.0f, MathHelper::Pi / 3.0f);
}

Camera::~Camera()
{
}

void Camera::SetProjection(float aspect, float nearPlane, float farPlane, float fieldOfView)
{
	m_Aspect = aspect;
	m_NearPlane = nearPlane;
	m_FarPlane = farPlane;
	m_FieldOfView = fieldOfView;

	XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(m_FieldOfView, m_Aspect, m_NearPlane, m_FarPlane);
	XMStoreFloat4x4(&m_Proj, projectionMatrix);
}

void Camera::UpdateCameraCBs()
{

}
