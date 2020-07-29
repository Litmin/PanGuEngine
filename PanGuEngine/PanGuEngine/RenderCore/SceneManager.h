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

	// ����SceneNode��Ĭ�ϵĸ��ڵ���RootNode
	SceneNode* CreateSceneNode();
	SceneNode* CreateSceneNode(SceneNode* parent);
	void DestroySceneNode(SceneNode* sceneNode);
	// �������еĽڵ��Transform
	void UpdateSceneNodeTransform();
	// ����ÿ��Renderer��Constant Buffer
	void UpdateRendererCBs();
	// ����Main Pass��Constant Buffer
	void UpdateMainPassBuffer();

	void AddMeshRenderer(MeshRenderer* meshRenderer);
	UINT GetRendererCount();
	void AddCamera(Camera* camera);
	void Render();
private:
	std::unique_ptr<SceneNode> m_RootNode;

	Camera* m_Camera;
	RTStateDesc m_RTState;
	// ������Ⱦ״̬����
	std::unordered_map<RendererStateDesc, std::vector<MeshRenderer*>> m_RenderQueue;
};

