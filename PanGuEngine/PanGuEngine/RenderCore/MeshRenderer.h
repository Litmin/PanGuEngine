#pragma once
#include "Mesh.h"
#include "Material.h"
#include "Renderer.h"

class MeshRenderer : public Renderer
{
public:
	MeshRenderer() = default;
	MeshRenderer(Material* material);

	// ���ݸ��µ�Constant Buffer���������֡��Դ��Buffer�������ˣ�Ҳ����NumFramesDirty==0�����ͷ���True��ʾ�������
	bool UpdateToConstantBuffer();

private:
	Mesh* m_Mesh;
	Material* m_Material;

	// PSO:MeshLayoutIndex��Shader

};

