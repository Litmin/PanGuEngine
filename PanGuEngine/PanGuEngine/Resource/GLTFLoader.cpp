#include "pch.h"
#include "GLTFLoader.h"
#include "Image.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

#include "Renderer/GameObject.h"
#include "Renderer/Material.h"
#include "D3D12RHI/GpuTexture2D.h"

namespace Resource
{
	GameObject* GLTFLoader::LoadGLTF(const std::string& filename, GameObject* sceneRoot)
	{
        assert(sceneRoot != nullptr);

		tinygltf::Model    gltf_model;
		tinygltf::TinyGLTF gltf_context;

        bool binary = false;
        size_t extpos = filename.rfind('.', filename.length());
        if (extpos != std::string::npos)
        {
            binary = (filename.substr(extpos + 1, filename.length() - extpos) == "glb");
        }

        std::string error;
        std::string warning;

        bool fileLoaded;
        if (binary)
            fileLoaded = gltf_context.LoadBinaryFromFile(&gltf_model, &error, &warning, filename.c_str());
        else
            fileLoaded = gltf_context.LoadASCIIFromFile(&gltf_model, &error, &warning, filename.c_str());
        if (!fileLoaded)
        {
            LOG_ERROR("Failed to load gltf file ");
            assert(0);
        }
        if (!warning.empty())
        {
            LOG_WARNING("Loaded gltf file ");
        }

        std::vector<std::shared_ptr<RHI::GpuTexture2D>> textures;
        std::vector<std::shared_ptr<Material>> materials;

        LoadTextures(gltf_model, textures);
        LoadMaterials(gltf_model, textures, materials);

        GameObject* glTFNode = sceneRoot->CreateChild();

        const tinygltf::Scene& scene = gltf_model.scenes[gltf_model.defaultScene > -1 ? gltf_model.defaultScene : 0];
        for (size_t i = 0; i < scene.nodes.size(); i++)
        {
            const tinygltf::Node node = gltf_model.nodes[scene.nodes[i]];
            LoadNode(glTFNode, node, scene.nodes[i], gltf_model);
        }

		return glTFNode;
	}


    void GLTFLoader::LoadTextures(const tinygltf::Model& gltf_model, std::vector<std::shared_ptr<RHI::GpuTexture2D>>& textures)
    {
        for (const tinygltf::Texture& gltf_tex : gltf_model.textures)
        {
            const tinygltf::Image& gltf_image = gltf_model.images[gltf_tex.source];

            if (gltf_image.width > 0 && gltf_image.height > 0)
            {
                std::shared_ptr<RHI::GpuTexture2D> texture2D = TextureFromGLTFImage(gltf_image);
                textures.push_back(texture2D);
            }
            else if (gltf_image.pixel_type == IMAGE_FILE_FORMAT_DDS || gltf_image.pixel_type == IMAGE_FILE_FORMAT_KTX)
            {
                assert(0 && "Not Implete for DDS, KTX.");
            }
        }
    }

    void GLTFLoader::LoadMaterials(const tinygltf::Model& gltf_model, std::vector<std::shared_ptr<RHI::GpuTexture2D>>& textures, 
                                   std::vector<std::shared_ptr<Material>>& materials)
    {
        for (const tinygltf::Material& gltf_mat : gltf_model.materials)
        {
            std::shared_ptr<RHI::GpuTexture2D> baseColorTex = nullptr;
			auto base_color_tex_it = gltf_mat.values.find("baseColorTexture");
            if (base_color_tex_it != gltf_mat.values.end())
            {
				baseColorTex = textures[base_color_tex_it->second.TextureIndex()];
				//Mat.TexCoordSets.BaseColor = static_cast<Uint8>(base_color_tex_it->second.TextureTexCoord());
            }

			std::shared_ptr<RHI::GpuTexture2D> metallicRoughnessTex = nullptr;
			auto metal_rough_tex_it = gltf_mat.values.find("metallicRoughnessTexture");
			if (metal_rough_tex_it != gltf_mat.values.end())
			{
                metallicRoughnessTex = textures[metal_rough_tex_it->second.TextureIndex()];
				//Mat.TexCoordSets.MetallicRoughness = static_cast<Uint8>(metal_rough_tex_it->second.TextureTexCoord());
			}

			std::shared_ptr<RHI::GpuTexture2D> normalTex = nullptr;
			auto normal_tex_it = gltf_mat.additionalValues.find("normalTexture");
			if (normal_tex_it != gltf_mat.additionalValues.end())
			{
                normalTex = textures[normal_tex_it->second.TextureIndex()];
				//Mat.TexCoordSets.Normal = static_cast<Uint8>(normal_tex_it->second.TextureTexCoord());
			}

			std::shared_ptr<RHI::GpuTexture2D> emissiveTex = nullptr;
			auto emssive_tex_it = gltf_mat.additionalValues.find("emissiveTexture");
			if (emssive_tex_it != gltf_mat.additionalValues.end())
			{
                emissiveTex = textures[emssive_tex_it->second.TextureIndex()];
				//Mat.TexCoordSets.Emissive = static_cast<Uint8>(emssive_tex_it->second.TextureTexCoord());
			}

			std::shared_ptr<RHI::GpuTexture2D> occlusionTex = nullptr;
			auto occlusion_tex_it = gltf_mat.additionalValues.find("occlusionTexture");
			if (occlusion_tex_it != gltf_mat.additionalValues.end())
			{
                occlusionTex = textures[occlusion_tex_it->second.TextureIndex()];
				//Mat.TexCoordSets.Occlusion = static_cast<Uint8>(occlusion_tex_it->second.TextureTexCoord());
			}

            float roughnessFactor = 1.0f;
			auto rough_factor_it = gltf_mat.values.find("roughnessFactor");
			if (rough_factor_it != gltf_mat.values.end())
			{
				roughnessFactor = static_cast<float>(rough_factor_it->second.Factor());
			}

            float metallicFactor = 1.0f;
			auto metal_factor_it = gltf_mat.values.find("metallicFactor");
			if (metal_factor_it != gltf_mat.values.end())
			{
				metallicFactor = static_cast<float>(metal_factor_it->second.Factor());
			}

            DirectX::XMFLOAT4 baseColorFactor = { 1.0f, 1.0f, 1.0f, 1.0f };
			auto base_col_factor_it = gltf_mat.values.find("baseColorFactor");
			if (base_col_factor_it != gltf_mat.values.end())
			{
                baseColorFactor = MakeVector4(base_col_factor_it->second.ColorFactor().data());
			}

            DirectX::XMFLOAT4 emissiveFactor = { 1.0f, 1.0f, 1.0f, 1.0f };
			auto emissive_fctr_it = gltf_mat.additionalValues.find("emissiveFactor");
			if (emissive_fctr_it != gltf_mat.additionalValues.end())
			{
                DirectX::XMFLOAT3 emissive3 = MakeVector3(emissive_fctr_it->second.ColorFactor().data());
                emissiveFactor = { emissive3.x, emissive3.y, emissive3.z, 1.0f };
				//Mat.EmissiveFactor = float4(0.0f);
			}

            std::shared_ptr<Material> material = std::make_shared<Material>(metallicFactor, roughnessFactor, baseColorFactor, emissiveFactor,
                                                                            baseColorTex, metallicRoughnessTex, normalTex, occlusionTex, emissiveTex);
            materials.push_back(material);
        }
    }

    void GLTFLoader::LoadNode(GameObject* parent, const tinygltf::Node& gltf_node, uint32_t nodeIndex, const tinygltf::Model& gltf_model)
    {
        assert(parent != nullptr);

        GameObject* gameObject = parent->CreateChild();

        if (gltf_node.translation.size() == 3)
        {
            gameObject->SetLocalPosition(MakeVector3(gltf_node.translation.data()));
        }

        if (gltf_node.rotation.size() == 4)
        {
            DirectX::XMFLOAT4 rotation = MakeVector4(gltf_node.rotation.data());
            gameObject->SetLocalRotation(Math::Quaternion(DirectX::XMLoadFloat4(&rotation)));
            //NewNode->rotation = glm::mat4(q);
        }

        if (gltf_node.scale.size() == 3)
        {
            gameObject->SetLocalScale(MakeVector3(gltf_node.scale.data()));
        }

        if (gltf_node.matrix.size() == 16)
        {
            LOG_ERROR("Not Supported.");
        }

        // Node with children
        if (gltf_node.children.size() > 0)
        {
            for (size_t i = 0; i < gltf_node.children.size(); i++)
            {
                LoadNode(gameObject, gltf_model.nodes[gltf_node.children[i]], gltf_node.children[i], gltf_model);
            }
        }

        if (gltf_node.mesh > -1)
        {
            const tinygltf::Mesh& gltf_mesh = gltf_model.meshes[gltf_node.mesh];
            for (size_t j = 0; j < gltf_mesh.primitives.size(); j++)
            {
                const tinygltf::Primitive& primitive = gltf_mesh.primitives[j];


            }
        }
    }


    std::shared_ptr<RHI::GpuTexture2D> GLTFLoader::TextureFromGLTFImage(const tinygltf::Image& gltfimage)
    {
        if (gltfimage.image.empty() || gltfimage.width <= 0 || gltfimage.height <= 0 || gltfimage.component <= 0)
        {
            assert(0 && "Failed to create texture for gltf_image.");
        }

        std::vector<UINT8> RGBA;

        const UINT8* pTextureData = nullptr;

        // 三个通道RGB
        if (gltfimage.component == 3)
        {
            RGBA.resize(gltfimage.width * gltfimage.height * 4);

            // Due to depressing performance of iterators in debug MSVC we have to use raw pointers here
            const auto* rgb = gltfimage.image.data();
            auto* rgba = RGBA.data();
            for (int i = 0; i < gltfimage.width * gltfimage.height; ++i)
            {
                rgba[0] = rgb[0];
                rgba[1] = rgb[1];
                rgba[2] = rgb[2];
                rgba[3] = 255;

                rgba += 4;
                rgb += 3;
            }
            assert(rgb == gltfimage.image.data() + gltfimage.image.size());
            assert(rgba == RGBA.data() + RGBA.size());

            pTextureData = RGBA.data();
        }
        // 四个通道RGBA
        else if (gltfimage.component == 4)
        {
            pTextureData = gltfimage.image.data();
        }

        std::shared_ptr<RHI::GpuTexture2D> texture2D = std::make_shared<RHI::GpuTexture2D>(gltfimage.width, gltfimage.height, DXGI_FORMAT_R8G8B8A8_UNORM,
                                                                                           4 * gltfimage.width, pTextureData);


        return texture2D;
    }
}

