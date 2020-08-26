#pragma once
#include "Mesh.h"
#include "Material.h"
#include "Renderer.h"
#include "RendererStateDesc.h"

extern const int gNumFrameResources;

class MeshRenderer : public Renderer
{
public:
	void SetMesh(Mesh* mesh);
	void SetMaterial(Material* material);
	void SetConstantBufferIndex(UINT index) { m_ObjCBIndex = index; }
	void UpdateRendererCBs();

	void Render(ID3D12GraphicsCommandList* commandList, ID3D12DescriptorHeap* cbvHeap, UINT rendererCount);

	const RendererStateDesc& GetRendererStateDesc();

private:
	void _UpdateRendererState();

private:
	Mesh* m_Mesh;
	Material* m_Material;
	std::unique_ptr<RendererStateDesc> m_StateDesc;
	bool m_RendererStateDirty = true;

	// 在Constant Buffer中的索引
	UINT m_ObjCBIndex = -1;
	int m_NumFramesDirty = gNumFrameResources;
};

