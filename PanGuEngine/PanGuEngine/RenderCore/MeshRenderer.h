#pragma once
#include "Mesh.h"
#include "Material.h"
#include "Renderer.h"

class MeshRenderer : public Renderer
{
public:
	MeshRenderer() = default;
	MeshRenderer(Material* material);

	// 数据更新到Constant Buffer，如果所有帧资源的Buffer都更新了（也就是NumFramesDirty==0），就返回True表示更新完成
	bool UpdateToConstantBuffer();

private:
	Mesh* m_Mesh;
	Material* m_Material;

	// PSO:MeshLayoutIndex和Shader

};

