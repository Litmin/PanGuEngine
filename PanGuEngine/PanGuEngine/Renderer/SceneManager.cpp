#include "pch.h"
#include "SceneManager.h"
#include "GameObject.h"
#include "GraphicContext.h"
#include "Input.h"

using namespace std;
using namespace DirectX;

extern const int gNumFrameResources;

SceneManager::SceneManager()
{
	// 创建场景根节点
	m_Root = make_unique<GameObject>(nullptr);
}

SceneManager::~SceneManager()
{

}

GameObject* SceneManager::CreateGameObject()
{
	return CreateGameObject(m_Root.get());
}

GameObject* SceneManager::CreateGameObject(GameObject* parent)
{
	return parent->CreateChild();
}


void SceneManager::DestroyGameObject(GameObject* gameObject)
{

}

void SceneManager::UpdateCameraMovement(float deltaTime)
{
	if (m_Camera == nullptr)
		return;

	GameObject* cameraGo = m_Camera->GetGameObject();

	static float moveSpeed = 10.0f;
	static float rotateSpeed = 3.0f;

	if (Input::GetKey(KeyCode::W))
	{
		cameraGo->Translate(0.0f, 0.0f, moveSpeed * deltaTime, Space::Self);
	}

	if (Input::GetKey(KeyCode::A))
	{
		cameraGo->Translate(-moveSpeed * deltaTime, 0.0f, 0.0f, Space::Self);
	}

	if (Input::GetKey(KeyCode::S))
	{
		cameraGo->Translate(0.0f, 0.0f, -moveSpeed * deltaTime, Space::Self);
	}

	if (Input::GetKey(KeyCode::D))
	{
		cameraGo->Translate(moveSpeed * deltaTime, 0.0f, 0.0f, Space::Self);
	}

	if (Input::GetKeyDown(KeyCode::Mouse0) || Input::GetKeyDown(KeyCode::Mouse2))
	{
		m_LastMousePos = Input::GetMousePosition();
	}

	if (Input::GetKey(KeyCode::Mouse0) || Input::GetKey(KeyCode::Mouse2))
	{
		XMINT2 curMousePos = Input::GetMousePosition();

		cameraGo->Rotate((float)(curMousePos.y - m_LastMousePos.y) * deltaTime * rotateSpeed, 
			(float)(curMousePos.x - m_LastMousePos.x) * deltaTime * rotateSpeed, 0.0f, Space::Self);

		m_LastMousePos = curMousePos;
	}
}

void SceneManager::AddMeshRenderer(MeshRenderer* meshRenderer)
{
	m_MeshRenderers.push_back(meshRenderer);
}


void SceneManager::AddCamera(Camera* camera)
{
	m_Camera = camera;
}
