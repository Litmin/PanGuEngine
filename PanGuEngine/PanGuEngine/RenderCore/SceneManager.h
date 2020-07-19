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

	// 创建SceneNode，默认的父节点是RootNode
	SceneNode* CreateSceneNode();
	SceneNode* CreateSceneNode(SceneNode* parent);

	void DestroySceneNode();

	// 更新所有的节点的Transform
	void UpdateSceneNodeTransform();

private:
	std::unique_ptr<SceneNode> m_RootNode;

	std::unique_ptr<RenderQueue> m_RenderQueue;
};

