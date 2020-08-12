#pragma once
#include "SceneManager.h"
#include "Component.h"

/*----------------------------------------------------------
	GameObject�ǳ�������֯��ʽ�����ɳ���ͼ
------------------------------------------------------------*/

class GameObject
{
public:
	GameObject(GameObject* parent);
	~GameObject();

	// �����ӽڵ�
	GameObject* CreateChild();
	// ɾ�������ӽڵ�
	void DestroyChildren();

	// �Լ���Transform
	const DirectX::XMFLOAT4X4& GetTransform();
	// �Լ������и��ڵ��Transform�����
	const DirectX::XMFLOAT4X4& GetCombinedTransform();
	// �����Լ��������ӽڵ��Transform
	void UpdateTransform();
	void Translate(float x, float y, float z);

	void AttachObject(Component* movableObject);

private:
	// ����������
	SceneManager* m_SceneManager;
	// ���ڵ�
	GameObject* m_Parent;
	// �ӽڵ�
	std::vector<std::unique_ptr<GameObject>> m_Children;

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
	std::vector<std::unique_ptr<Component>> m_Components;
};

