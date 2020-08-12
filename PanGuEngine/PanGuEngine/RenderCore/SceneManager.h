#pragma once
#include "Light.h"
#include "MeshRenderer.h"
#include "Camera.h"
#include "RenderQueue.h"
#include "RendererStateDesc.h"

class GameObject;
class SceneManager : public Singleton<SceneManager>
{
public:
	SceneManager();
	~SceneManager();

	GameObject* GetRootNode() { return m_Root.get(); }

	// 创建SceneNode，默认的父节点是RootNode
	GameObject* CreateGameObject();
	GameObject* CreateGameObject(GameObject* parent);
	void DestroyGameObject(GameObject* gameObject);
	// 更新所有的节点的Transform
	void UpdateTransform();
	void BuildConstantBuffer();
	// 更新每个Renderer的Constant Buffer
	void UpdateRendererCBs();
	// 更新Main Pass的Constant Buffer
	void UpdateMainPassBuffer();

	void AddMeshRenderer(MeshRenderer* meshRenderer);
	UINT GetRendererCount();
	void AddCamera(Camera* camera);
	void Render();
private:
	std::unique_ptr<GameObject> m_Root;

	Camera* m_Camera;
	RTStateDesc m_RTState;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RendererAndPassCBVHeap;
	// 按照渲染状态排序
	std::unordered_map<RendererStateDesc, std::vector<MeshRenderer*>> m_RenderQueue;
};

