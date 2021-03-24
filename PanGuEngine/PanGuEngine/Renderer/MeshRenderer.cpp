#include "pch.h"
#include "MeshRenderer.h"
#include "GraphicContext.h"
#include "GameObject.h"

using namespace std;
using namespace DirectX;


void MeshRenderer::SetMesh(Mesh* mesh)
{
	m_Mesh = mesh;
	m_RendererStateDirty = true;
}

void MeshRenderer::SetMaterial(Material* material)
{
	m_Material = material;
	m_RendererStateDirty = true;
}

void MeshRenderer::UpdateRendererCBs()
{
	auto currObjectCB = GraphicContext::GetSingleton().GetCurrFrameResource()->m_ObjectCB.get();
	// Only update the cbuffer data if the constants have changed.  
	// This needs to be tracked per frame resource.
	if (m_NumFramesDirty > 0)
	{
		XMMATRIX world = XMLoadFloat4x4(&m_GameObject->LocalToWorldMatrix());

		ObjectConstants objConstants;
		XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));

		currObjectCB->CopyData(m_ObjCBIndex, objConstants);

		// Next FrameResource need to be updated too.
		m_NumFramesDirty--;
	}
}

void MeshRenderer::Render(ID3D12GraphicsCommandList* commandList, ID3D12DescriptorHeap* cbvHeap, UINT rendererCount)
{
	commandList->IASetVertexBuffers(0, 1, &m_Mesh->VertexBufferView());
	commandList->IASetIndexBuffer(&m_Mesh->IndexBufferView());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT curFrameResourceIndex = GraphicContext::GetSingleton().GetCurrFrameResourceIndex();
	UINT cbvSrvUavDescriptorSize = GraphicContext::GetSingleton().GetCbvSrvUavDescriptorSize();

	// Offset to the CBV in the descriptor heap for this object and for this frame resource.
	UINT cbvIndex = curFrameResourceIndex * rendererCount + m_ObjCBIndex;
	auto cbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(cbvHeap->GetGPUDescriptorHandleForHeapStart());
	cbvHandle.Offset(cbvIndex, cbvSrvUavDescriptorSize);

	m_StateDesc->shaderPtr->SetDescriptorTable(commandList, ShaderManager::GetSingleton().PropertyToID("cbPerObject"), cbvHandle);

	commandList->DrawIndexedInstanced(m_Mesh->IndexCount(), 1, 0, 0, 0);
}

const RendererStateDesc& MeshRenderer::GetRendererStateDesc()
{
	_UpdateRendererState();

	return *m_StateDesc.get();
}

void MeshRenderer::_UpdateRendererState()
{
	if (m_RendererStateDirty && m_Mesh != nullptr && m_Material != nullptr)
	{
		m_RendererStateDirty = false;

		m_StateDesc = make_unique<RendererStateDesc>
			(
				m_Material->GetShader(),
				m_Mesh->GetLayoutIndex(),
				D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
			);
	}
}
