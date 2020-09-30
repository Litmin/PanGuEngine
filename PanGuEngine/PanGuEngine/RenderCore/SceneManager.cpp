#include "pch.h"
#include "SceneManager.h"
#include "GameObject.h"
#include "GraphicContext.h"
#include "Input.h"

using namespace std;
using namespace DirectX;

extern const int gNumFrameResources;

SceneManager::SceneManager()
{
	// 创建场景根节点
	m_Root = make_unique<GameObject>(nullptr);

	m_RTState.depthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	m_RTState.rtCount = 1;
	m_RTState.rtFormat[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
}

SceneManager::~SceneManager()
{

}

GameObject* SceneManager::CreateGameObject()
{
	return CreateGameObject(m_Root.get());
}

GameObject* SceneManager::CreateGameObject(GameObject* parent)
{
	return parent->CreateChild();
}


void SceneManager::DestroyGameObject(GameObject* gameObject)
{

}

void SceneManager::BuildConstantBuffer()
{
	UINT objCount = GetRendererCount();

	// 每个Renderer需要一个CBV，+1是PerPass的CBV
	UINT numDescriptors = (objCount + 1) * gNumFrameResources;

	// PerPass的CBV开始的位置
	UINT mPassCbvOffset = objCount * gNumFrameResources;

	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = numDescriptors;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	ThrowIfFailed(GraphicContext::GetSingleton().Device()->CreateDescriptorHeap(&cbvHeapDesc,
		IID_PPV_ARGS(&m_RendererAndPassCBVHeap)));

	// 每个Renderer的CBV
	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	// Need a CBV descriptor for each object for each frame resource.
	for (int frameIndex = 0; frameIndex < gNumFrameResources; ++frameIndex)
	{
		auto objectCB = GraphicContext::GetSingleton().GetFrameResource(frameIndex)->m_ObjectCB->Resource();
		for (UINT i = 0; i < objCount; ++i)
		{
			D3D12_GPU_VIRTUAL_ADDRESS cbAddress = objectCB->GetGPUVirtualAddress();

			// Offset to the ith object constant buffer in the buffer.
			cbAddress += i * objCBByteSize;

			// Offset to the object cbv in the descriptor heap.
			int heapIndex = frameIndex * objCount + i;
			auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_RendererAndPassCBVHeap->GetCPUDescriptorHandleForHeapStart());
			handle.Offset(heapIndex, GraphicContext::GetSingleton().GetCbvSrvUavDescriptorSize());

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
			cbvDesc.BufferLocation = cbAddress;
			cbvDesc.SizeInBytes = objCBByteSize;

			GraphicContext::GetSingleton().Device()->CreateConstantBufferView(&cbvDesc, handle);
		}
	}

	// MainPass的CBV
	UINT passCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants));
	// Last three descriptors are the pass CBVs for each frame resource.
	for (int frameIndex = 0; frameIndex < gNumFrameResources; ++frameIndex)
	{
		auto passCB = GraphicContext::GetSingleton().GetFrameResource(frameIndex)->m_PassCB->Resource();
		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = passCB->GetGPUVirtualAddress();

		// Offset to the pass cbv in the descriptor heap.
		int heapIndex = mPassCbvOffset + frameIndex;
		auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_RendererAndPassCBVHeap->GetCPUDescriptorHandleForHeapStart());
		handle.Offset(heapIndex, GraphicContext::GetSingleton().GetCbvSrvUavDescriptorSize());

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = passCBByteSize;

		GraphicContext::GetSingleton().Device()->CreateConstantBufferView(&cbvDesc, handle);
	}

	// MeshRenderer的CBV index
	UINT objCBVIndex = 0;
	for (auto& [rendererStateDesc, meshRenderers] : m_RenderQueue)
	{
		for (auto meshRenderer : meshRenderers)
		{
			meshRenderer->SetConstantBufferIndex(objCBVIndex);
			objCBVIndex++;
		}
	}
}

void SceneManager::UpdateCameraMovement(float deltaTime)
{
	if (m_Camera == nullptr)
		return;

	GameObject* cameraGo = m_Camera->GetGameObject();

	static float moveSpeed = 10.0f;
	static float rotateSpeed = 3.0f;

	if (Input::GetKey(KeyCode::W))
	{
		cameraGo->Translate(0.0f, 0.0f, moveSpeed * deltaTime, Space::Self);
	}

	if (Input::GetKey(KeyCode::A))
	{
		cameraGo->Translate(-moveSpeed * deltaTime, 0.0f, 0.0f, Space::Self);
	}

	if (Input::GetKey(KeyCode::S))
	{
		cameraGo->Translate(0.0f, 0.0f, -moveSpeed * deltaTime, Space::Self);
	}

	if (Input::GetKey(KeyCode::D))
	{
		cameraGo->Translate(moveSpeed * deltaTime, 0.0f, 0.0f, Space::Self);
	}

	if (Input::GetKeyDown(KeyCode::Mouse0) || Input::GetKeyDown(KeyCode::Mouse2))
	{
		m_LastMousePos = Input::GetMousePosition();
	}

	if (Input::GetKey(KeyCode::Mouse0) || Input::GetKey(KeyCode::Mouse2))
	{
		XMINT2 curMousePos = Input::GetMousePosition();

		cameraGo->Rotate((float)(curMousePos.y - m_LastMousePos.y) * deltaTime * rotateSpeed, 
			(float)(curMousePos.x - m_LastMousePos.x) * deltaTime * rotateSpeed, 0.0f, Space::Self);

		m_LastMousePos = curMousePos;
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
		auto commandList = GraphicContext::GetSingleton().CommandList();

		// 设置Pipeline State
		auto pso = GraphicContext::GetSingleton().GetPSO(rendererStateDesc, m_RTState);
		commandList->SetPipelineState(pso);
		
		// 绑定CBV Heap
		ID3D12DescriptorHeap* descriptorHeaps[] = { m_RendererAndPassCBVHeap.Get() };
		commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

		// 绑定根签名
		rendererStateDesc.shaderPtr->BindRootSignature(commandList);

		// 绑定MainPass CB
		int passCbvIndex = GetRendererCount() * gNumFrameResources + GraphicContext::GetSingleton().GetCurrFrameResourceIndex();
		auto passCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_RendererAndPassCBVHeap->GetGPUDescriptorHandleForHeapStart());
		passCbvHandle.Offset(passCbvIndex, GraphicContext::GetSingleton().GetCbvSrvUavDescriptorSize());
		rendererStateDesc.shaderPtr->SetDescriptorTable(commandList, ShaderManager::GetSingleton().PropertyToID("cbPass"), passCbvHandle);

		// 渲染每个MeshRenderer
		for (auto meshRenderer : meshRenderers)
		{
			meshRenderer->Render(commandList, m_RendererAndPassCBVHeap.Get(), GetRendererCount());
		}
	}
}
