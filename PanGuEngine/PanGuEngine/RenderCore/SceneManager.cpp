#include "pch.h"
#include "SceneManager.h"

SceneManager::SceneManager()
{
	// 创建场景根节点

}

SceneManager::~SceneManager()
{

}

SceneNode* SceneManager::GetRootNode()
{
	return m_RootNode.get();
}

SceneNode* SceneManager::CreateSceneNode()
{
	return CreateSceneNode(m_RootNode.get());
}

SceneNode* SceneManager::CreateSceneNode(SceneNode* parent)
{
	return nullptr;
}


void SceneManager::DestroySceneNode()
{
}

void SceneManager::UpdateSceneNodeTransform()
{
}
