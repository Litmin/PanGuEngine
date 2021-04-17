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

	std::shared_ptr<RHI::GpuTexture2D> GetDefaultWhiteTex() { return m_DefaultWhiteTex; }
	std::shared_ptr<RHI::GpuTexture2D> GetDefaultBlackTex() { return m_DefaultBlackTex; }

private:
    // Default Texture
    std::shared_ptr<RHI::GpuTexture2D> m_DefaultWhiteTex = nullptr;
    std::shared_ptr<RHI::GpuTexture2D> m_DefaultBlackTex = nullptr;
};



