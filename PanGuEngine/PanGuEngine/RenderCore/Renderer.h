#pragma once
#include "Mesh.h"

class Renderer
{
public:
	Renderer() = default;
	Renderer(const Renderer& rhs) = delete;

private:
	Mesh* m_Mesh;

	DirectX::XMFLOAT4X4 m_WorldMatrix = MathHelper::Identity4x4();

	D3D12_PRIMITIVE_TOPOLOGY m_PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	UINT m_IndexCount = 0;
	UINT m_StartIndexLocation = 0;
	int m_BaseVertexLocation = 0;
};

