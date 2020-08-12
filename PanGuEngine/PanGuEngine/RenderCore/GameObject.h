#pragma once
#include "SceneManager.h"
#include "Component.h"

/*----------------------------------------------------------
	GameObject是场景的组织方式，构成场景图
------------------------------------------------------------*/

class GameObject
{
public:
	GameObject(GameObject* parent);
	~GameObject();

	// 创建子节点
	GameObject* CreateChild();
	// 删除所有子节点
	void DestroyChildren();

	// 自己的Transform
	const DirectX::XMFLOAT4X4& GetTransform();
	// 自己和所有父节点的Transform的组合
	const DirectX::XMFLOAT4X4& GetCombinedTransform();
	// 更新自己和所有子节点的Transform
	void UpdateTransform();
	void Translate(float x, float y, float z);

	void AttachObject(Component* movableObject);

private:
	// 场景管理器
	SceneManager* m_SceneManager;
	// 父节点
	GameObject* m_Parent;
	// 子节点
	std::vector<std::unique_ptr<GameObject>> m_Children;

	// TODO:
	// Position
	// Rotation
	// Scale
	// Right
	// Up
	// Forward
	// 模型空间到世界空间的变换矩阵
	DirectX::XMFLOAT4X4 m_Transform;
	// 自己和所有父节点的Transform的组合
	DirectX::XMFLOAT4X4 m_CombinedTransform;
	// 每个物体的Constant Buffer索引
	UINT m_ObjCBIndex = -1;

	// Components
	std::vector<std::unique_ptr<Component>> m_Components;
};

