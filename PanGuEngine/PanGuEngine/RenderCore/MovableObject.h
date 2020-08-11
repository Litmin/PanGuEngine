#pragma once

class SceneNode;
// 表示场景中可移动的物体，可以挂接到SceneNode
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

