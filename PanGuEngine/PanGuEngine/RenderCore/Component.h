#pragma once

class GameObject;
// ��ʾ�����п��ƶ������壬���Թҽӵ�SceneNode
class Component
{
public:
	Component() = default;
	virtual ~Component() = default;

	void SetGameObject(GameObject* gameObject);

	// TODO:OnAttachedEvent: Update Camera View Matrix

protected:
	GameObject* m_GameObject = nullptr;
};

