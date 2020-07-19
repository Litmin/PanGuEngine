#pragma once
#include "Light.h"
#include "MeshRenderer.h"
#include "Camera.h"
#include "RenderQueue.h"
#include "SceneNode.h"

class SceneManager : public Singleton<SceneManager>
{
public:
	SceneManager();
	~SceneManager();

	SceneNode* GetRootNode();

	// ����SceneNode��Ĭ�ϵĸ��ڵ���RootNode
	SceneNode* CreateSceneNode();
	SceneNode* CreateSceneNode(SceneNode* parent);

	void DestroySceneNode();

	// �������еĽڵ��Transform
	void UpdateSceneNodeTransform();

private:
	std::unique_ptr<SceneNode> m_RootNode;

	std::unique_ptr<RenderQueue> m_RenderQueue;
};

