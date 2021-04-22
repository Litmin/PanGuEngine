#include "pch.h"
#include "Camera.h"
#include "GameObject.h"

using namespace DirectX;

Camera::Camera()
{
	m_View = MathHelper::Identity4x4();
	m_Proj = MathHelper::Identity4x4();
}

Camera::~Camera()
{
}

void Camera::OnAddToGameObject()
{
	SceneManager::GetSingleton().AddCamera(this);
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

std::array<DirectX::XMVECTOR, 8> Camera::GetFrustumCorners(float overrideFarPlane)
{
	std::array<DirectX::XMVECTOR, 8> corners;

	float nearPlaneHalfHeight = m_NearPlane * Math::Tan(m_FieldOfView / 2.0f);
	float nearPlaneHalfWidth = nearPlaneHalfHeight * m_Aspect;

	float farPlaneHalfHeight = overrideFarPlane == 0.0f ? m_FarPlane / m_NearPlane * nearPlaneHalfHeight : overrideFarPlane / m_NearPlane * nearPlaneHalfHeight;
	float farPlaneHalfWidth = farPlaneHalfHeight * m_Aspect;

	DirectX::XMVECTOR nearPlaneCorner0 = XMVectorSet(-nearPlaneHalfWidth, -nearPlaneHalfHeight, m_NearPlane, 1.0f);
	DirectX::XMVECTOR nearPlaneCorner1 = XMVectorSet(nearPlaneHalfWidth, -nearPlaneHalfHeight, m_NearPlane, 1.0f);
	DirectX::XMVECTOR nearPlaneCorner2 = XMVectorSet(-nearPlaneHalfWidth, nearPlaneHalfHeight, m_NearPlane, 1.0f);
	DirectX::XMVECTOR nearPlaneCorner3 = XMVectorSet(nearPlaneHalfWidth, nearPlaneHalfHeight, m_NearPlane, 1.0f);

	DirectX::XMVECTOR farPlaneCorner0 = XMVectorSet(-farPlaneHalfWidth, -farPlaneHalfHeight, m_FarPlane, 1.0f);
	DirectX::XMVECTOR farPlaneCorner1 = XMVectorSet(farPlaneHalfWidth, -farPlaneHalfHeight, m_FarPlane, 1.0f);
	DirectX::XMVECTOR farPlaneCorner2 = XMVectorSet(-farPlaneHalfWidth, farPlaneHalfHeight, m_FarPlane, 1.0f);
	DirectX::XMVECTOR farPlaneCorner3 = XMVectorSet(farPlaneHalfWidth, farPlaneHalfHeight, m_FarPlane, 1.0f);

	DirectX::XMMATRIX inverseView = XMLoadFloat4x4(&m_GameObject->LocalToWorldMatrix());

	// 变换到世界空间
	corners[0] = XMVector4Transform(nearPlaneCorner0, inverseView);
	corners[1] = XMVector4Transform(nearPlaneCorner1, inverseView);
	corners[2] = XMVector4Transform(nearPlaneCorner2, inverseView);
	corners[3] = XMVector4Transform(nearPlaneCorner3, inverseView);

	corners[4] = XMVector4Transform(farPlaneCorner0, inverseView);
	corners[5] = XMVector4Transform(farPlaneCorner1, inverseView);
	corners[6] = XMVector4Transform(farPlaneCorner2, inverseView);
	corners[7] = XMVector4Transform(farPlaneCorner3, inverseView);

	return corners;
}

void Camera::UpdateCameraCBs(void* perPassCB, const DirectX::XMFLOAT4X4& shadowViewProj)
{
	assert(perPassCB);

	m_View = m_GameObject->LocalToWorldMatrix();

	XMMATRIX view = XMLoadFloat4x4(&m_View);
	view = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX proj = XMLoadFloat4x4(&m_Proj);


	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	XMStoreFloat4x4(&m_PerPassCBData.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&m_PerPassCBData.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&m_PerPassCBData.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&m_PerPassCBData.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&m_PerPassCBData.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&m_PerPassCBData.InvViewProj, XMMatrixTranspose(invViewProj));
	m_PerPassCBData.ShadowViewProj = shadowViewProj;

	m_PerPassCBData.EyePosW = XMFLOAT3(m_GameObject->WorldPosition().GetX(), m_GameObject->WorldPosition().GetY(), m_GameObject->WorldPosition().GetZ());
	m_PerPassCBData.NearZ = m_NearPlane;
	m_PerPassCBData.FarZ = m_FarPlane;
	//m_MainPassCB.RenderTargetSize = XMFLOAT2((float)mClientWidth, (float)mClientHeight);
	//m_MainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
	//m_MainPassCB.TotalTime = gt.TotalTime();
	//m_MainPassCB.DeltaTime = gt.DeltaTime();

	memcpy(perPassCB, &m_PerPassCBData, sizeof(PerPassConstants));
}
