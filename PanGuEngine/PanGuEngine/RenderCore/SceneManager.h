#pragma once
#include "Light.h"
#include "MeshRenderer.h"
#include "Camera.h"
#include "RenderQueue.h"
#include "RendererStateDesc.h"

class SceneNode;
class SceneManager : public Singleton<SceneManager>
{
public:
	SceneManager();
	~SceneManager();

	SceneNode* GetRootNode() { return m_RootNode.get(); }

	// 创建SceneNode，默认的父节点是RootNode
	SceneNode* CreateSceneNode();
	SceneNode* CreateSceneNode(SceneNode* parent);
	void DestroySceneNode(SceneNode* sceneNode);
	// 更新所有的节点的Transform
	void UpdateSceneNodeTransform();
	// 更新每个Renderer的Constant Buffer
	void UpdateRendererCBs();
	// 更新Main Pass的Constant Buffer
	void UpdateMainPassBuffer();

	void AddMeshRenderer(MeshRenderer* meshRenderer);
	UINT GetRendererCount();
	void AddCamera(Camera* camera);
	void Render();
private:
	std::unique_ptr<SceneNode> m_RootNode;

	Camera* m_Camera;
	RTStateDesc m_RTState;
	// 按照渲染状态排序
	std::unordered_map<RendererStateDesc, std::vector<MeshRenderer*>> m_RenderQueue;
};

