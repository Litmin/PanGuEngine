#pragma once

class GameObject;
// 表示场景中可移动的物体，可以挂接到SceneNode
class Component
{
public:
	Component() = default;
	virtual ~Component() = default;

	void SetGameObject(GameObject* gameObject);
	GameObject* GetGameObject() { return m_GameObject; }

	// TODO:OnAttachedEvent: Update Camera View Matrix

protected:
	GameObject* m_GameObject = nullptr;
};

