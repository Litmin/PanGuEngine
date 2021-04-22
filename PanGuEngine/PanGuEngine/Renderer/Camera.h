#pragma once
#include "Component.h"

extern const int gNumFrameResources;
class Camera : public Component
{
public:
	Camera();
	virtual ~Camera();

	virtual void OnAddToGameObject();

	void SetProjection(float aspect, float nearPlane, float farPlane, float fieldOfView);
	// ����ռ�����׶��İ˸���������
	std::array<DirectX::XMVECTOR, 8> GetFrustumCorners(float overrideFarPlane = 0.0f);
	void UpdateCameraCBs(void* perPassCB, const DirectX::XMFLOAT4X4& shadowViewProj);

protected:
	float m_Aspect;
	float m_NearPlane;
	float m_FarPlane;

	// TODO:�������
	// ͸�����
	float m_FieldOfView;

	// Matrix
	DirectX::XMFLOAT4X4 m_View;// = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 m_Proj;// = MathHelper::Identity4x4();

	PerPassConstants m_PerPassCBData;
};

