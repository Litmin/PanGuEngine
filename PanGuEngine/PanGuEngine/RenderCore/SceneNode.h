#pragma once
#include "SceneManager.h"
#include "MovableObject.h"

class SceneNode
{
public:
	SceneNode(SceneNode* parent);
	~SceneNode();

	// �����ӽڵ�
	SceneNode* CreateChildNode();
	// ɾ�������ӽڵ�
	void DestroyChildNode();

	// �Լ���Transform
	const DirectX::XMFLOAT4X4& GetTransform();
	// �Լ������и��ڵ��Transform�����
	const DirectX::XMFLOAT4X4& GetCombinedTransform();
	// �����Լ��������ӽڵ��Transform
	void UpdateTransform();

	void AttachObject(MovableObject* movableObject);

private:
	// ����������
	SceneManager* m_SceneManager;
	// ���ڵ�
	SceneNode* m_Parent;
	// �ӽڵ�
	std::vector<std::unique_ptr<SceneNode>> m_Children;

	// TODO:
	// Position
	// Rotation
	// Scale
	// Right
	// Up
	// Forward
	// ģ�Ϳռ䵽����ռ�ı任����
	DirectX::XMFLOAT4X4 m_Transform;
	// �Լ������и��ڵ��Transform�����
	DirectX::XMFLOAT4X4 m_CombinedTransform;
	// ÿ�������Constant Buffer����
	UINT m_ObjCBIndex = -1;

	// Components
	std::vector<std::unique_ptr<MovableObject>> m_Components;
};

