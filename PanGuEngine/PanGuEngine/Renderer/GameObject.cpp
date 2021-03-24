#include "pch.h"
#include "GameObject.h"
#include "MeshRenderer.h"

using namespace std;
using namespace DirectX;
using namespace Math;

GameObject::GameObject(GameObject* parent) :
	m_Parent(parent)
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

void GameObject::AttachObject(Component* movableObject)
{
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

void GameObject::Translate(float x, float y, float z, Space relativeTo)
{
	m_TransformDirty = true;

	if (relativeTo == Space::Self)
	{
		m_Position += (m_Rotation * Vector3(x, y, z));
	}
	else if(relativeTo == Space::World)
	{
		_UpdateFromParent();

		if (m_Parent)
		{
			m_Position += (~m_Parent->m_DerivedRotation * Vector3(x, y, z)) / m_Parent->m_DerivedScale;
		}
		else
		{
			m_Position += Vector3(x, y, z);
		}
	}
}

void GameObject::Rotate(float xAngle, float yAngle, float zAngle, Space relativeTo)
{
	m_TransformDirty = true;

	if (relativeTo == Space::Self)
	{
		m_Rotation = m_Rotation * Quaternion(XMConvertToRadians(xAngle), XMConvertToRadians(yAngle), XMConvertToRadians(zAngle));
	}
	else if (relativeTo == Space::World)
	{
		_UpdateFromParent();

		m_Rotation = m_Rotation * ~m_DerivedRotation * Quaternion(XMConvertToRadians(xAngle), XMConvertToRadians(yAngle), XMConvertToRadians(zAngle)) * m_DerivedRotation;
	}
}

DirectX::XMFLOAT4X4 GameObject::LocalToWorldMatrix()
{
	_UpdateFromParent();

	return m_LocalToWorldMatrix;
}

void GameObject::_UpdateFromParent()
{
	if (m_TransformDirty)
	{
		m_TransformDirty = false;

		Vector3 parentScale(1.0f, 1.0f, 1.0f);
		Quaternion parentRotation;
		Vector3 parentPosition(0.0f, 0.0f, 0.0f);

		if (m_Parent)
		{
			m_Parent->_UpdateFromParent();
			parentScale = m_Parent->m_DerivedScale;
			parentRotation = m_Parent->m_DerivedRotation;
			parentPosition = m_Parent->m_DerivedPosition;
		}

		m_DerivedScale = parentScale * m_Scale;
		m_DerivedRotation = parentRotation * m_Rotation;
		// 子节点的Position跟父节点的Scale、Rotation有关
		m_DerivedPosition = parentRotation * (parentScale * m_Position);
		m_DerivedPosition += parentPosition;

		// TODO:优化，去掉三个矩阵的乘法，只需要从四元数构造一个矩阵
		XMMATRIX scale = XMMatrixScalingFromVector(m_DerivedScale);
		XMMATRIX rotate = XMMatrixRotationQuaternion(m_DerivedRotation);
		XMMATRIX translation = XMMatrixTranslationFromVector(m_DerivedPosition);

		XMMATRIX localToWorldMatrix = scale * rotate * translation;
		XMStoreFloat4x4(&m_LocalToWorldMatrix, localToWorldMatrix);
	}
}
