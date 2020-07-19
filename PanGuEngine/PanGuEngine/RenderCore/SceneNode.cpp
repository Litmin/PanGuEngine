#include "pch.h"
#include "SceneNode.h"

using namespace DirectX;

SceneNode::SceneNode(SceneNode* parent, SceneManager* sceneManager) :
	m_SceneManager(sceneManager),
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

	// 更新所有子节点的Transform

}
