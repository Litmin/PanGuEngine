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

	void SetConstantBufferIndex(UINT index) { m_ObjCBIndex = index; }
	void UpdateRendererCBs();

	void Render(ID3D12GraphicsCommandList* commandList, ID3D12DescriptorHeap* cbvHeap, UINT rendererCount);

	const RendererStateDesc& GetRendererStateDesc() { return *m_StateDesc.get(); }
private:
	Mesh* m_Mesh;
	Material* m_Material;
	std::unique_ptr<RendererStateDesc> m_StateDesc;


	// 在Constant Buffer中的索引
	UINT m_ObjCBIndex = -1;
	int m_NumFramesDirty = gNumFrameResources;
};

