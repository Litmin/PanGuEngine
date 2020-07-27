#include "pch.h"
#include "SceneNode.h"
#include "MeshRenderer.h"

using namespace DirectX;
using namespace std;

SceneNode::SceneNode(SceneNode* parent) :
	m_Parent(parent),
	m_Transform(MathHelper::Identity4x4()),
	m_CombinedTransform(MathHelper::Identity4x4())
{
}

SceneNode::~SceneNode()
{

}

SceneNode* SceneNode::CreateChildNode()
{
	return nullptr;
}

void SceneNode::DestroyChildNode()
{
}

const DirectX::XMFLOAT4X4& SceneNode::GetTransform()
{
	return m_Transform;
}

const DirectX::XMFLOAT4X4& SceneNode::GetCombinedTransform()
{
	return m_CombinedTransform;
}

void SceneNode::UpdateTransform()
{
	if (m_Parent)
	{
		XMMATRIX parentCombinedMatrix = XMLoadFloat4x4(&m_Parent->GetCombinedTransform());
		XMMATRIX selfMatrix = XMLoadFloat4x4(&m_Transform);

		XMStoreFloat4x4(&m_CombinedTransform, XMMatrixMultiply(parentCombinedMatrix, selfMatrix));
	}
	else
	{
		m_CombinedTransform = m_Transform;
	}

	// TODO:更新所有子节点的Transform

}

void SceneNode::AttachObject(MovableObject* movableObject)
{
	//m_Components.push_back((make_unique<MovableObject>(*movableObject)));
	m_Components.push_back(unique_ptr<MovableObject>(movableObject));
	
	MeshRenderer* meshRenderer = static_cast<MeshRenderer*>(movableObject);
	if (nullptr != meshRenderer)
		SceneManager::GetSingleton().AddMeshRenderer(meshRenderer);

	Camera* camera = static_cast<Camera*>(movableObject);
	if (camera != nullptr)
	{

	}
}
