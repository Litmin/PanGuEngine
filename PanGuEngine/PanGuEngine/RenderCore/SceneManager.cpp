#include "pch.h"
#include "SceneManager.h"

SceneManager::SceneManager()
{
	// �����������ڵ�

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
