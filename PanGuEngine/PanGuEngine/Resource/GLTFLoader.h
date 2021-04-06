#pragma once

namespace tinygltf
{
	struct Image;
	class Node;
	class Model;
}

namespace RHI
{
	class GpuTexture2D;
}

class GameObject;
class Material;


namespace Resource
{

	class GLTFLoader
	{
	public:
		static GameObject* LoadGLTF(const std::string& filename, GameObject* root);

	private:
		static void LoadTextures(const tinygltf::Model& gltf_model, std::vector<std::shared_ptr<RHI::GpuTexture2D>>& textures);

		static void LoadMaterials(const tinygltf::Model& gltf_model, std::vector<std::shared_ptr<RHI::GpuTexture2D>>& textures, 
								  std::vector<std::shared_ptr<Material>>& materials);
			
		static std::shared_ptr<RHI::GpuTexture2D> TextureFromGLTFImage(const tinygltf::Image& gltfimage);


		template <typename Y>
		static DirectX::XMFLOAT3 MakeVector3(const Y& vals)
		{
			return DirectX::XMFLOAT3 //
			{
				static_cast<T>(vals[0]),
				static_cast<T>(vals[1]),
				static_cast<T>(vals[2]) //
			};
		}

		template <typename Y>
		static DirectX::XMFLOAT4 MakeVector4(const Y& vals)
		{
			return DirectX::XMFLOAT4 
			{
				static_cast<T>(vals[0]),
				static_cast<T>(vals[1]),
				static_cast<T>(vals[2]),
				static_cast<T>(vals[3]) 
			};
		}
	};

}


