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

	// ����SceneNode��Ĭ�ϵĸ��ڵ���RootNode
	GameObject* CreateGameObject();
	GameObject* CreateGameObject(GameObject* parent);
	void DestroyGameObject(GameObject* gameObject);
	// �������еĽڵ��Transform
	void UpdateTransform();
	void BuildConstantBuffer();
	// ����ÿ��Renderer��Constant Buffer
	void UpdateRendererCBs();
	// ����Main Pass��Constant Buffer
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
	// ������Ⱦ״̬����
	std::unordered_map<RendererStateDesc, std::vector<MeshRenderer*>> m_RenderQueue;
};

