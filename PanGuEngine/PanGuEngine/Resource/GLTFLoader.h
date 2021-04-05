#pragma once

namespace tinygltf
{
	struct Image;
	class Node;
	class Model;
}

class GameObject;
namespace RHI
{
	class GpuTexture2D;
}

namespace Resource
{

	class GLTFLoader
	{
	public:
		static GameObject* LoadGLTF(const std::string& filename, GameObject* root);

	private:
		static void LoadTextures(const tinygltf::Model& gltf_model, std::vector<std::shared_ptr<RHI::GpuTexture2D>>& textures);

		static void LoadMaterials(const tinygltf::Model& gltf_model, std::vector<std::shared_ptr<RHI::GpuTexture2D>>& textures);
			
		static std::shared_ptr<RHI::GpuTexture2D> TextureFromGLTFImage(const tinygltf::Image& gltfimage);
	};

}


