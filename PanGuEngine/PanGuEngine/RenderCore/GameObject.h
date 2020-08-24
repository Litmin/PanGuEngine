#pragma once
#include "SceneManager.h"
#include "Component.h"
#include "Math/Vector.h"
#include "Math/Quaternion.h"

/*
------------------------------------------------------------
	GameObject�ǳ�������֯��ʽ�����ɳ���ͼ
------------------------------------------------------------
*/

enum class Space
{
	World,
	Self
};

class GameObject
{
public:
	GameObject(GameObject* parent);
	~GameObject();

	// �����ӽڵ�
	GameObject* CreateChild();
	// ɾ�������ӽڵ�
	void DestroyChildren();

	void AttachObject(Component* movableObject);

	void Translate(float x, float y, float z, Space relativeTo = Space::Self);
	void Translate(DirectX::XMFLOAT3 translation, Space relativeTo = Space::Self);
	void Rotate(DirectX::XMFLOAT3 eulers, Space relativeTo = Space::Self);
	void Rotate(float xAngle, float yAngle, float zAngle, Space relativeTo = Space::Self);
	void Rotate(DirectX::XMFLOAT3 axis, float angle, Space relativeTo = Space::Self);

	Math::Vector3 LocalPosition() const { return m_Position; }
	Math::Vector3 WorldPosition() const { return m_DerivedPosition; }
	Math::Quaternion LocalRotation() const { return m_Rotation; }
	Math::Quaternion WorldRotation() const { return m_DerivedRotation; }
	Math::Vector3 LocalScale() const { return m_Scale; }
	Math::Vector3 WorldScale() const { return m_DerivedScale; }

	DirectX::XMFLOAT4X4 LocalToWorldMatrix();

private:
	void _UpdateFromParent();

private:
	// ����������
	SceneManager* m_SceneManager;
	// ���ڵ�
	GameObject* m_Parent;
	// �ӽڵ�
	std::vector<std::unique_ptr<GameObject>> m_Children;

	// Transform
	Math::Vector3 m_Position;
	Math::Quaternion m_Rotation;
	Math::Vector3 m_Scale;
	Math::Vector3 m_DerivedPosition;
	Math::Quaternion m_DerivedRotation;
	Math::Vector3 m_DerivedScale;
	
	DirectX::XMFLOAT4X4 m_LocalToWorldMatrix;
	DirectX::XMFLOAT4X4 m_WorldToLocalMatrix;

	bool m_TransformDirty = true;
	// ÿ�������Constant Buffer����
	UINT m_ObjCBIndex = -1;

	// Components
	std::vector<std::unique_ptr<Component>> m_Components;
};

