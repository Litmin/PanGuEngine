#include "pch.h"
#include "SceneManager.h"
#include "GameObject.h"
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
	static float rotateSpeed = 40.0f;

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

	if (Input::GetKey(KeyCode::Q))
	{
		cameraGo->Translate(0.0f, -moveSpeed * deltaTime, 0.0f, Space::Self);
	}

	if (Input::GetKey(KeyCode::E))
	{
		cameraGo->Translate(0.0f, moveSpeed * deltaTime, 0.0f, Space::Self);
	}

	if (Input::GetKeyDown(KeyCode::Mouse0) || Input::GetKeyDown(KeyCode::Mouse2))
	{
		m_LastMousePos = Input::GetMousePosition();
	}
	
	if (Input::GetKey(KeyCode::Mouse0) || Input::GetKey(KeyCode::Mouse2))
	{
		XMINT2 curMousePos = Input::GetMousePosition();

		m_LastRotation.x += XMConvertToRadians((float)(curMousePos.y - m_LastMousePos.y) * deltaTime * rotateSpeed);
		m_LastRotation.y += XMConvertToRadians((float)(curMousePos.x - m_LastMousePos.x) * deltaTime * rotateSpeed);

		Math::Quaternion newRotation(m_LastRotation.x, m_LastRotation.y, 0.0f);
		cameraGo->SetLocalRotation(newRotation);

		m_LastMousePos = curMousePos;
	}

	if (Input::GetKeyDown(KeyCode::P))
	{
		Math::Quaternion newRotation(0.0f, XMConvertToRadians(10.0f), 0.0f);
		cameraGo->SetLocalRotation(newRotation);
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

void SceneManager::AddLight(Light* light)
{
	m_Light = light;
}
