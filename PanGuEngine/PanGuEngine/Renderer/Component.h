#pragma once

class GameObject;

class Component
{
public:
	Component() = default;
	virtual ~Component() = default;

	void SetGameObject(GameObject* gameObject);
	GameObject* GetGameObject() { return m_GameObject; }

	//virtual void _OnTransformUpdate();
	virtual void OnAddToGameObject() {}

protected:
	GameObject* m_GameObject = nullptr;
};

