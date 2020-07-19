#pragma once
#include "Mesh.h"
#include "Material.h"
#include "Renderer.h"

class MeshRenderer : public Renderer
{
public:
	MeshRenderer() = default;
	MeshRenderer(DirectX::XMFLOAT4X4 transform, PBRMaterial* material);

	// 数据更新到Constant Buffer，如果所有帧资源的Buffer都更新了（也就是NumFramesDirty==0），就返回True表示更新完成
	bool UpdateToConstantBuffer();

private:
	Mesh* m_Mesh;
	PBRMaterial* m_Material;

	DirectX::XMFLOAT4X4 m_Transform = MathHelper::Identity4x4();
	UINT m_ObjConstantIndex;

	int m_NumFramesDirty;
};

