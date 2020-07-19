#pragma once
#include "RenderCore/Mesh.h"

class ResourceManager : public Singleton<ResourceManager>
{
public:
    std::shared_ptr<Mesh> LoadMesh(std::string path);

    void UnloadUnusedResources();

private:
    std::unordered_map<std::string, std::shared_ptr<Mesh>> m_Meshes;
};

