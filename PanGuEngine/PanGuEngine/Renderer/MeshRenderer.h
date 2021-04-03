#pragma once
#include "Mesh.h"
#include "Material.h"
#include "Renderer.h"
#include "RendererStateDesc.h"


class MeshRenderer : public Renderer
{
public:
	void SetMesh(Mesh* mesh);

	void Render(RHI::GraphicsContext& graphicContext, void* perDrawCB) const;

private:
	Mesh* m_Mesh;
};

