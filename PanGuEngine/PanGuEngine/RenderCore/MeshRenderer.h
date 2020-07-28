#pragma once
#include "Mesh.h"
#include "Material.h"
#include "Renderer.h"
#include "RendererStateDesc.h"

extern const int gNumFrameResources;

class MeshRenderer : public Renderer
{
public:
	MeshRenderer(Mesh* mesh, Material* material);

	// ���ݸ��µ�Constant Buffer���������֡��Դ��Buffer�������ˣ�Ҳ����NumFramesDirty==0�����ͷ���True��ʾ�������
	bool UpdateRendererCBs();

	const RendererStateDesc& GetRendererStateDesc() { return *m_StateDesc.get(); }
private:
	Mesh* m_Mesh;
	Material* m_Material;
	std::unique_ptr<RendererStateDesc> m_StateDesc;


	// ��Constant Buffer�е�����
	UINT m_ObjCBIndex = -1;
	int m_NumFramesDirty = gNumFrameResources;
};

