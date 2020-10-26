#pragma once

class GameObject;
// ��ʾ�����п��ƶ������壬���Թҽӵ�SceneNode
class Component
{
public:
	Component() = default;
	virtual ~Component() = default;

	void SetGameObject(GameObject* gameObject);
	GameObject* GetGameObject() { return m_GameObject; }

	//virtual void _OnTransformUpdate();

protected:
	GameObject* m_GameObject = nullptr;
};

