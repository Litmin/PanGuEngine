#include "pch.h"
#include "GameObject.h"
#include "MeshRenderer.h"

using namespace DirectX;
using namespace std;

GameObject::GameObject(GameObject* parent) :
	m_Parent(parent),
	m_Transform(MathHelper::Identity4x4()),
	m_CombinedTransform(MathHelper::Identity4x4())
{
}

GameObject::~GameObject()
{

}

GameObject* GameObject::CreateChild()
{
	GameObject* child = new GameObject(this);
	m_Children.push_back(unique_ptr<GameObject>(child));
	return child;
}

void GameObject::DestroyChildren()
{
}

const DirectX::XMFLOAT4X4& GameObject::GetTransform()
{
	return m_Transform;
}

const DirectX::XMFLOAT4X4& GameObject::GetCombinedTransform()
{
	return m_CombinedTransform;
}

void GameObject::UpdateTransform()
{
	if (m_Parent)
	{
		XMMATRIX parentCombinedMatrix = XMLoadFloat4x4(&m_Parent->GetCombinedTransform());
		XMMATRIX selfMatrix = XMLoadFloat4x4(&m_Transform);

		XMStoreFloat4x4(&m_CombinedTransform, XMMatrixMultiply(parentCombinedMatrix, selfMatrix));
	}
	else
	{
		m_CombinedTransform = m_Transform;
	}

	for (auto& child : m_Children)
	{
		child->UpdateTransform();
	}
}

void GameObject::Translate(float x, float y, float z)
{
	XMMATRIX newTransform = XMLoadFloat4x4(&m_Transform) * XMMatrixTranslation(x, y, z);
	XMStoreFloat4x4(&m_Transform, newTransform);
}

void GameObject::AttachObject(Component* movableObject)
{
	//m_Components.push_back((make_unique<MovableObject>(*movableObject)));
	m_Components.push_back(unique_ptr<Component>(movableObject));
	
	// TODO:Remove dynamic_cast
	MeshRenderer* meshRenderer = dynamic_cast<MeshRenderer*>(movableObject);
	if (nullptr != meshRenderer)
	{
		SceneManager::GetSingleton().AddMeshRenderer(meshRenderer);
	}

	Camera* camera = dynamic_cast<Camera*>(movableObject);
	if (nullptr != camera)
	{
		SceneManager::GetSingleton().AddCamera(camera);
	}
}
