#include "pch.h"
#include "ResourceManager.h"

namespace Resource
{

	std::shared_ptr<Mesh> ResourceManager::LoadMesh(std::string path)
	{
		return std::shared_ptr<Mesh>();
	}

	void ResourceManager::UnloadUnusedResources()
	{
		// 引用计数大于1的为在使用的资源
	}

}

