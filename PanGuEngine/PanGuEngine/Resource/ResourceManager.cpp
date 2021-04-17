#include "pch.h"
#include "ResourceManager.h"
#include "D3D12RHI/GpuTexture2D.h"

ResourceManager::ResourceManager()
{
	std::vector<UINT8> whiteTexData = { 255, 255, 255, 255,
									   255, 255, 255, 255,
									   255, 255, 255, 255,
									   255, 255, 255, 255 };
	m_DefaultWhiteTex = std::make_shared<RHI::GpuTexture2D>(2, 2, DXGI_FORMAT_R8G8B8A8_UNORM, 4 * 2, whiteTexData.data());

	std::vector<UINT8> blackTexData = { 0, 0, 0, 255,
										0, 0, 0, 255,
										0, 0, 0, 255,
										0, 0, 0, 255 };
	m_DefaultBlackTex = std::make_shared<RHI::GpuTexture2D>(2, 2, DXGI_FORMAT_R8G8B8A8_UNORM, 4 * 2, blackTexData.data());
}

