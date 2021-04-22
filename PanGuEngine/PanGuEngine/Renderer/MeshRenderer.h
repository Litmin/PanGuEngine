#pragma once
#include "Mesh.h"
#include "Material.h"
#include "Renderer.h"


class MeshRenderer : public Renderer
{
public:
	virtual void OnAddToGameObject() override;

	void SetMesh(std::shared_ptr<Mesh> mesh);
	void SetMaterial(std::shared_ptr<Material> material);

	Material* GetMaterial() { return m_Material.get(); }

	void Render(RHI::GraphicsContext& graphicContext, void* perDrawCB, bool useMaterial = true) const;

private:
	std::shared_ptr<Mesh> m_Mesh = nullptr;
	std::shared_ptr<Material> m_Material = nullptr;
};

