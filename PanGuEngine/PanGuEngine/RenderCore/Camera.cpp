#include "pch.h"
#include "Camera.h"
#include "GraphicContext.h"

using namespace DirectX;

Camera::Camera()
{
	m_View = MathHelper::Identity4x4();
	m_Proj = MathHelper::Identity4x4();

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
	XMMATRIX view = XMLoadFloat4x4(&m_View);
	XMMATRIX proj = XMLoadFloat4x4(&m_Proj);

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	XMStoreFloat4x4(&m_MainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&m_MainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&m_MainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&m_MainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&m_MainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&m_MainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));

	//m_MainPassCB.EyePosW = mEyePos;
	//m_MainPassCB.RenderTargetSize = XMFLOAT2((float)mClientWidth, (float)mClientHeight);
	//m_MainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
	m_MainPassCB.NearZ = m_NearPlane;
	m_MainPassCB.FarZ = m_FarPlane;
	//m_MainPassCB.TotalTime = gt.TotalTime();
	//m_MainPassCB.DeltaTime = gt.DeltaTime();

	auto currPassCB = GraphicContext::GetSingleton().GetCurrFrameResource()->m_PassCB.get();
	currPassCB->CopyData(0, m_MainPassCB);
}
