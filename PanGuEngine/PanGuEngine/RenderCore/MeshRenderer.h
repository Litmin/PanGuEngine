#pragma once
#include "Mesh.h"

class MeshRenderer
{
public:
	MeshRenderer() = default;
	MeshRenderer(const MeshRenderer& rhs) = delete;

private:
	Mesh* m_Mesh;

	DirectX::XMFLOAT4X4 m_WorldMatrix = MathHelper::Identity4x4();
};

