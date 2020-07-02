#pragma once
#include "Light.h"
#include "MeshRenderer.h"
#include "Camera.h"

class Scene
{
public:
	void AddRenderer();
	void AddLight();

private:
	std::vector<Light*> m_Lights;
	std::vector<MeshRenderer*> m_Renderers;
};

