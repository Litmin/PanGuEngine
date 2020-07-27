#include "pch.h"
#include "SceneManager.h"

using namespace std;

SceneManager::SceneManager()
{
	// 创建场景根节点
	m_RootNode = make_unique<SceneNode>(nullptr, this);
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

void SceneManager::AddMeshRenderer(MeshRenderer* meshRenderer)
{
	RendererStateDesc rendererStateDesc = meshRenderer->GetRendererStateDesc();
	m_RenderQueue[rendererStateDesc].push_back(meshRenderer);
}

void SceneManager::AddCamera(Camera* camera)
{
	m_Cameras.push_back(camera);
}

void SceneManager::Render()
{
	// for each camera
	for (auto camera : m_Cameras)
	{
		// Update Camera Constant

		// for each rendererState
		for()

		//
	}
}
