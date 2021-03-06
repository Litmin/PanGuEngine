#include "pch.h"
#include "MeshRenderer.h"
#include "GameObject.h"
#include "D3D12RHI/CommandContext.h"

using namespace std;
using namespace DirectX;
using namespace RHI;


void MeshRenderer::OnAddToGameObject()
{
	SceneManager::GetSingleton().AddMeshRenderer(this);
}

void MeshRenderer::SetMesh(std::shared_ptr<Mesh> mesh)
{
	m_Mesh = mesh;
}

void MeshRenderer::SetMaterial(std::shared_ptr<Material> material)
{
	m_Material = material;
}


void MeshRenderer::Render(GraphicsContext& graphicContext, void* perDrawCB, bool useMaterial) const
{
	assert(perDrawCB != nullptr);
	assert(m_Mesh != nullptr);
	assert(m_Material != nullptr);

	graphicContext.SetVertexBuffer(0, m_Mesh->VertexBufferView());
	graphicContext.SetIndexBuffer(m_Mesh->IndexBufferView());
	graphicContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	if(useMaterial)
		graphicContext.SetShaderResourceBinding(m_Material->GetSRB());

	PerDrawConstants perDrawData;
	XMMATRIX objectToWorld = XMLoadFloat4x4(&m_GameObject->LocalToWorldMatrix());
	XMMATRIX worldToObject = XMMatrixInverse(nullptr, objectToWorld);
	XMStoreFloat4x4(&perDrawData.ObjectToWorld, XMMatrixTranspose(objectToWorld));
	XMStoreFloat4x4(&perDrawData.WorldToObject, XMMatrixTranspose(worldToObject));
	memcpy(perDrawCB, &perDrawData, sizeof(PerDrawConstants));

	graphicContext.DrawIndexedInstanced(m_Mesh->IndexCount(), 1, 0, 0, 0);
}

