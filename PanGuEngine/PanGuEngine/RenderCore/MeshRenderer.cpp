#include "pch.h"
#include "MeshRenderer.h"
#include "GraphicContext.h"

using namespace std;

MeshRenderer::MeshRenderer(Mesh* mesh, Material* material) :
	m_Mesh(mesh),
	m_Material(material)
{
	m_StateDesc = make_unique<RendererStateDesc>
	(
		m_Material->GetShader(),
		m_Mesh->GetLayoutIndex(),
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
	);
}

bool MeshRenderer::UpdateRendererCBs()
{
	return true;
}
