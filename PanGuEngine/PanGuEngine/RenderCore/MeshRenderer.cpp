#include "pch.h"
#include "MeshRenderer.h"

MeshRenderer::MeshRenderer(DirectX::XMFLOAT4X4 transform, PBRMaterial* material) :
	m_Transform{transform},
	m_Material{material},
	m_ObjConstantIndex{-1},
	m_NumFramesDirty{0}
{

}

bool MeshRenderer::UpdateToConstantBuffer()
{
	if (m_NumFramesDirty > 0)
	{
		m_NumFramesDirty--;
		
		return false;
	}

	return true;
}
