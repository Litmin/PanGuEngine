#include "pch.h"
#include "GLTFLoader.h"
#include "Image.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

#include "Renderer/GameObject.h"
#include "D3D12RHI/GpuTexture2D.h"

namespace Resource
{
	GameObject* GLTFLoader::LoadGLTF(const std::string& filename, GameObject* root)
	{
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


        LoadTextures(gltf_model, textures);


		return nullptr;
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

    void GLTFLoader::LoadMaterials(const tinygltf::Model& gltf_model, std::vector<std::shared_ptr<RHI::GpuTexture2D>>& textures)
    {
        for (const tinygltf::Material& gltf_mat : gltf_model.materials)
        {

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

