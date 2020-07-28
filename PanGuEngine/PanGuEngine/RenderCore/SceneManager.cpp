#include "pch.h"
#include "SceneManager.h"
#include "SceneNode.h"

using namespace std;

SceneManager::SceneManager()
{
	// 创建场景根节点
	m_RootNode = make_unique<SceneNode>(nullptr);
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
}

void SceneManager::AddCamera(Camera* camera)
{
	m_Camera = camera;
}

void SceneManager::Render()
{

}
