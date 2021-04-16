#pragma once
#include "Light.h"
#include "MeshRenderer.h"
#include "Camera.h"
#include "D3D12RHI/CommandContext.h"

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
	void UpdateCameraMovement(float deltaTime);

	void AddMeshRenderer(MeshRenderer* meshRenderer);
	void AddCamera(Camera* camera);

	Camera* GetCamera() { return m_Camera; }
	const std::vector<MeshRenderer*>& GetDrawList() const { return m_MeshRenderers; }

private:
	std::unique_ptr<GameObject> m_Root;

	Camera* m_Camera;
	DirectX::XMINT2 m_LastMousePos;
	DirectX::XMFLOAT2 m_LastRotation;

	// TODO:空间分割，按渲染状态排序
	std::vector<MeshRenderer*> m_MeshRenderers;
};

