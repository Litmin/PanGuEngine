#include "pch.h"
#include "Component.h"
#include "GameObject.h"

void Component::SetGameObject(GameObject* gameObject)
{
	m_GameObject = gameObject;
}
