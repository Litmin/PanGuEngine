#pragma once
#include "SceneManager.h"
#include "Component.h"
#include "Math/Vector.h"
#include "Math/Quaternion.h"


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

	// 创建子节点
	GameObject* CreateChild();
	// 删除所有子节点
	void DestroyChildren();

	template<typename T> T* AddComponent();
	template<typename T> T* GetComponent();

	void Translate(float x, float y, float z, Space relativeTo = Space::Self);
	void Translate(DirectX::XMFLOAT3 translation, Space relativeTo = Space::Self);
	void Rotate(DirectX::XMFLOAT3 eulers, Space relativeTo = Space::Self);
	void Rotate(float xAngle, float yAngle, float zAngle, Space relativeTo = Space::Self);
	void Rotate(DirectX::XMFLOAT3 axis, float angle, Space relativeTo = Space::Self);

	void SetLocalPosition(Math::Vector3 localPosition) { m_TransformDirty = true; m_Position = localPosition; }
	void SetLocalRotation(Math::Quaternion localRotation) { m_TransformDirty = true; m_Rotation = localRotation; }
	void SetLocalScale(Math::Vector3 localScale) { m_TransformDirty = true; m_Scale = localScale; }

	Math::Vector3 LocalPosition() const { return m_Position; }
	Math::Vector3 WorldPosition() const { return m_DerivedPosition; }
	Math::Quaternion LocalRotation() const { return m_Rotation; }
	Math::Quaternion WorldRotation() const { return m_DerivedRotation; }
	Math::Vector3 LocalScale() const { return m_Scale; }
	Math::Vector3 WorldScale() const { return m_DerivedScale; }

	DirectX::XMFLOAT4X4 LocalToWorldMatrix();

private:
	void _UpdateFromParent();

	// 父节点
	GameObject* m_Parent;
	// 子节点
	std::vector<std::unique_ptr<GameObject>> m_Children;

	// Transform
	Math::Vector3 m_Position = { 0.0f,0.0f,0.0f };
	Math::Quaternion m_Rotation;
	Math::Vector3 m_Scale = { 1.0f,1.0f,1.0f };
	Math::Vector3 m_DerivedPosition;
	Math::Quaternion m_DerivedRotation;
	Math::Vector3 m_DerivedScale;
	
	DirectX::XMFLOAT4X4 m_LocalToWorldMatrix;
	DirectX::XMFLOAT4X4 m_WorldToLocalMatrix;

	bool m_TransformDirty = true;
	// 每个物体的Constant Buffer索引
	UINT m_ObjCBIndex = -1;

	// Components
	std::vector<std::unique_ptr<Component>> m_Components;
};

template<typename T>
inline T* GameObject::AddComponent()
{
	static_assert(std::is_base_of<Component, T>::value,
		"T must be classes derived from Component..");

	T* component = GetComponent<T>();

	if (component != nullptr)
	{
		LOG_WARNING("Warning: Already have Component");
		return component;
	}

	component = new T();
	component->SetGameObject(this);
	m_Components.push_back(std::unique_ptr<T>(component));
	return component;
}

template<typename T>
inline T* GameObject::GetComponent()
{
	for (int i = 0; i < m_Components.size(); ++i)
	{
		Component* baseComponent = m_Components[i].get();
		T* concreteComponent = dynamic_cast<T*>(baseComponent);
		if (concreteComponent != nullptr)
			return concreteComponent;
	}

	return nullptr;
}
