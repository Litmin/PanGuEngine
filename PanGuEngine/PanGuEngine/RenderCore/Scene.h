#pragma once
#include "Light.h"
#include "MeshRenderer.h"

class Scene
{
public:

private:
	std::vector<Light> m_Lights;
	std::vector<MeshRenderer> m_Renderers;
};

