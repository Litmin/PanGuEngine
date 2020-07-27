#pragma once
#include "Mesh.h"
#include "Material.h"
#include "Renderer.h"
#include "RendererStateDesc.h"

class MeshRenderer : public Renderer
{
public:
	MeshRenderer(Mesh* mesh, Material* material);

	// 数据更新到Constant Buffer，如果所有帧资源的Buffer都更新了（也就是NumFramesDirty==0），就返回True表示更新完成
	bool UpdateToConstantBuffer();

	const RendererStateDesc& GetRendererStateDesc() { return *m_StateDesc.get(); }
private:
	Mesh* m_Mesh;
	Material* m_Material;

	std::unique_ptr<RendererStateDesc> m_StateDesc;
};

