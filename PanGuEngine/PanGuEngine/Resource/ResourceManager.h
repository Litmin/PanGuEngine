#pragma once
#include "Renderer/Mesh.h"

namespace RHI
{
    class GpuTexture2D;
}


class ResourceManager : public Singleton<ResourceManager>
{
public:
    ResourceManager();


private:
    // Default Texture
    std::shared_ptr<RHI::GpuTexture2D> m_DefaultWhiteTex = nullptr;
    std::shared_ptr<RHI::GpuTexture2D> m_BlackWhiteTex = nullptr;
};



