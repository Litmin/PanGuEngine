#pragma once

class SceneNode;
// ��ʾ�����п��ƶ������壬���Թҽӵ�SceneNode
class MovableObject
{
public:
	MovableObject() = default;
	virtual ~MovableObject() = default;

	void SetNode(SceneNode* node);

	// TODO:OnAttachedEvent: Update Camera View Matrix

protected:
	SceneNode* m_Node = nullptr;
};

