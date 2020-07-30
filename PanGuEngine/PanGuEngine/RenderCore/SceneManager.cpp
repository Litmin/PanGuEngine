#include "pch.h"
#include "SceneManager.h"
#include "SceneNode.h"
#include "GraphicContext.h"

using namespace std;

extern const int gNumFrameResources;

SceneManager::SceneManager()
{
	// �����������ڵ�
	m_RootNode = make_unique<SceneNode>(nullptr);

	m_RTState.depthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	m_RTState.rtCount = 1;
	m_RTState.rtFormat[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
}

SceneManager::~SceneManager()
{

}

SceneNode* SceneManager::CreateSceneNode()
{
	return CreateSceneNode(m_RootNode.get());
}

SceneNode* SceneManager::CreateSceneNode(SceneNode* parent)
{
	return parent->CreateChildNode();
}


void SceneManager::DestroySceneNode(SceneNode* sceneNode)
{

}

void SceneManager::UpdateSceneNodeTransform()
{
	m_RootNode->UpdateTransform();
}

void SceneManager::BuildConstantBuffer()
{
	UINT objCount = GetRendererCount();

	// Need a CBV descriptor for each object for each frame resource,
	// +1 for the perPass CBV for each frame resource.
	UINT numDescriptors = (objCount + 1) * gNumFrameResources;

	// Save an offset to the start of the pass CBVs.  These are the last 3 descriptors.
	UINT mPassCbvOffset = objCount * gNumFrameResources;

	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = numDescriptors;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	ThrowIfFailed(GraphicContext::GetSingleton().Device()->CreateDescriptorHeap(&cbvHeapDesc,
		IID_PPV_ARGS(&m_RendererAndPassCBVHeap)));


	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	// Need a CBV descriptor for each object for each frame resource.
	for (int frameIndex = 0; frameIndex < gNumFrameResources; ++frameIndex)
	{
		auto objectCB = mFrameResources[frameIndex]->ObjectCB->Resource();
		for (UINT i = 0; i < objCount; ++i)
		{
			D3D12_GPU_VIRTUAL_ADDRESS cbAddress = objectCB->GetGPUVirtualAddress();

			// Offset to the ith object constant buffer in the buffer.
			cbAddress += i * objCBByteSize;

			// Offset to the object cbv in the descriptor heap.
			int heapIndex = frameIndex * objCount + i;
			auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mCbvHeap->GetCPUDescriptorHandleForHeapStart());
			handle.Offset(heapIndex, mCbvSrvUavDescriptorSize);

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
			cbvDesc.BufferLocation = cbAddress;
			cbvDesc.SizeInBytes = objCBByteSize;

			md3dDevice->CreateConstantBufferView(&cbvDesc, handle);
		}
	}

}

void SceneManager::UpdateRendererCBs()
{
	for (auto [rendererStateDesc, meshRenderers] : m_RenderQueue)
	{
		for (auto meshRenderer : meshRenderers)
		{
			meshRenderer->UpdateRendererCBs();
		}
	}
}

void SceneManager::UpdateMainPassBuffer()
{
	m_Camera->UpdateCameraCBs();
}

void SceneManager::AddMeshRenderer(MeshRenderer* meshRenderer)
{
	RendererStateDesc rendererStateDesc = meshRenderer->GetRendererStateDesc();
	m_RenderQueue[rendererStateDesc].push_back(meshRenderer);
}

UINT SceneManager::GetRendererCount()
{
	UINT count = 0;
	for (auto& [rendererStateDesc, meshRenderers] : m_RenderQueue)
	{
		count += meshRenderers.size();
	}

	return count;
}

void SceneManager::AddCamera(Camera* camera)
{
	m_Camera = camera;
}

void SceneManager::Render()
{
	for (auto& [rendererStateDesc, meshRenderers] : m_RenderQueue)
	{
		auto pso = GraphicContext::GetSingleton().GetPSO(rendererStateDesc, m_RTState);
		GraphicContext::GetSingleton().CommandList()->SetPipelineState(pso);
		

	}
}
