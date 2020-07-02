#pragma once
#include "Mesh.h"
#include "Material.h"

class MeshRenderer
{
public:
	MeshRenderer() = default;
	MeshRenderer(DirectX::XMFLOAT4X4 transform, PBRMaterial* material);
	MeshRenderer(const MeshRenderer& rhs) = delete;

	// ���ݸ��µ�Constant Buffer���������֡��Դ��Buffer�������ˣ�Ҳ����NumFramesDirty==0�����ͷ���True��ʾ�������
	bool UpdateToConstantBuffer();

private:
	Mesh* m_Mesh;
	PBRMaterial* m_Material;

	DirectX::XMFLOAT4X4 m_Transform = MathHelper::Identity4x4();
	UINT m_ObjConstantIndex;

	int m_NumFramesDirty;
};

