#pragma once
#include "Renderer/Mesh.h"

namespace Resource
{

    class ResourceManager : public Singleton<ResourceManager>
    {
    public:
        std::shared_ptr<Mesh> LoadMesh(std::string path);

        void UnloadUnusedResources();

    private:
        std::unordered_map<std::string, std::shared_ptr<Mesh>> m_Meshes;
    };

}


