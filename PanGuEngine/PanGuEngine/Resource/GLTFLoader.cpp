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
            LoadNode(glTFNode, node, scene.nodes[i], gltf_model, materials);
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

    void GLTFLoader::LoadNode(GameObject* parent, const tinygltf::Node& gltf_node, uint32_t nodeIndex, const tinygltf::Model& gltf_model, 
                              std::vector<std::shared_ptr<Material>>& materials)
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
                LoadNode(gameObject, gltf_model.nodes[gltf_node.children[i]], gltf_node.children[i], gltf_model, materials);
            }
        }

        if (gltf_node.mesh > -1)
        {
            const tinygltf::Mesh& gltf_mesh = gltf_model.meshes[gltf_node.mesh];
            for (size_t j = 0; j < gltf_mesh.primitives.size(); j++)
            {
                const tinygltf::Primitive& primitive = gltf_mesh.primitives[j];

                GameObject* rendererGO = parent->CreateChild();

				uint32_t indexCount = 0;
				uint32_t vertexCount = 0;

				const float* bufferPos = nullptr;
				const float* bufferTexCoordSet0 = nullptr;
				const float* bufferNormals = nullptr;
				const float* bufferTangents = nullptr;

				int posStride = -1;
				int texCoordSet0Stride = -1;
				int normalsStride = -1;
                int tangentsStride = -1;

				auto position_it = primitive.attributes.find("POSITION");
				assert(position_it != primitive.attributes.end() && "Position attribute is required");

				const tinygltf::Accessor& posAccessor = gltf_model.accessors[position_it->second];
				const tinygltf::BufferView& posView = gltf_model.bufferViews[posAccessor.bufferView];
				assert(posAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT && "Position component type is expected to be float");
				assert(posAccessor.type == TINYGLTF_TYPE_VEC3 && "Position type is expected to be vec3");

				bufferPos = reinterpret_cast<const float*>(&(gltf_model.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]));

                // 应该是3
				posStride = posAccessor.ByteStride(posView) / tinygltf::GetComponentSizeInBytes(posAccessor.componentType);
				assert(posStride > 0 && "Position stride is invalid");

				vertexCount = static_cast<uint32_t>(posAccessor.count);

				if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
				{
					const tinygltf::Accessor& uvAccessor = gltf_model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
					const tinygltf::BufferView& uvView = gltf_model.bufferViews[uvAccessor.bufferView];
					assert(uvAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT && "UV0 component type is expected to be float");
					assert(uvAccessor.type == TINYGLTF_TYPE_VEC2 && "UV0 type is expected to be vec2");

					bufferTexCoordSet0 = reinterpret_cast<const float*>(&(gltf_model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
					texCoordSet0Stride = uvAccessor.ByteStride(uvView) / tinygltf::GetComponentSizeInBytes(uvAccessor.componentType);
					assert(texCoordSet0Stride > 0 && "Texcoord0 stride is invalid");
				}

				if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
				{
					const tinygltf::Accessor& normAccessor = gltf_model.accessors[primitive.attributes.find("NORMAL")->second];
					const tinygltf::BufferView& normView = gltf_model.bufferViews[normAccessor.bufferView];
					assert(normAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT && "Normal component type is expected to be float");
					assert(normAccessor.type == TINYGLTF_TYPE_VEC3 && "Normal type is expected to be vec3");

					bufferNormals = reinterpret_cast<const float*>(&(gltf_model.buffers[normView.buffer].data[normAccessor.byteOffset + normView.byteOffset]));
					normalsStride = normAccessor.ByteStride(normView) / tinygltf::GetComponentSizeInBytes(normAccessor.componentType);
					assert(normalsStride > 0 && "Normal stride is invalid");
				}

				if (primitive.attributes.find("TANGENT") != primitive.attributes.end())
				{
					const tinygltf::Accessor& tangentAccessor = gltf_model.accessors[primitive.attributes.find("TANGENT")->second];
					const tinygltf::BufferView& tangentView = gltf_model.bufferViews[tangentAccessor.bufferView];
					assert(tangentAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT && "Tangent component type is expected to be float");
					assert(tangentAccessor.type == TINYGLTF_TYPE_VEC4 && "Tangent type is expected to be vec4");

					bufferTangents = reinterpret_cast<const float*>(&(gltf_model.buffers[tangentView.buffer].data[tangentAccessor.byteOffset + tangentView.byteOffset]));
					tangentsStride = tangentAccessor.ByteStride(tangentView) / tinygltf::GetComponentSizeInBytes(tangentAccessor.componentType);
					assert(tangentsStride > 0 && "Tangent stride is invalid");
				}
                else
                {
                    // TODO: 计算切线 http://www.mikktspace.com/
                }

				bool     hasIndices = primitive.indices > -1;
                std::vector<UINT32> indexBuffer;
                if (hasIndices)
                {
					const tinygltf::Accessor& accessor = gltf_model.accessors[primitive.indices > -1 ? primitive.indices : 0];
					const tinygltf::BufferView& bufferView = gltf_model.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = gltf_model.buffers[bufferView.buffer];

					indexCount = static_cast<UINT32>(accessor.count);

					const void* dataPtr = &(buffer.data[accessor.byteOffset + bufferView.byteOffset]);

					switch (accessor.componentType)
					{
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
					{
						const UINT32* buf = static_cast<const UINT32*>(dataPtr);
						for (size_t index = 0; index < accessor.count; index++)
						{
							indexBuffer.push_back(buf[index]);
						}
						break;
					}
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
					{
						const uint16_t* buf = static_cast<const uint16_t*>(dataPtr);
						for (size_t index = 0; index < accessor.count; index++)
						{
							indexBuffer.push_back(buf[index]);
						}
						break;
					}
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
					{
						const uint8_t* buf = static_cast<const uint8_t*>(dataPtr);
						for (size_t index = 0; index < accessor.count; index++)
						{
							indexBuffer.push_back(buf[index]);
						}
						break;
					}
					default:
						std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
						return;
					}

					MeshRenderer* meshRenderer = rendererGO->AddComponent<MeshRenderer>();
					std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(vertexCount, bufferPos, nullptr, bufferNormals, nullptr, bufferTexCoordSet0,
						nullptr, nullptr, nullptr, indexCount, indexBuffer.data());
                    meshRenderer->SetMesh(mesh);
                    meshRenderer->SetMaterial(primitive.material > -1 ? materials[primitive.material] : materials.back());

                    // TODO: 在Component的Add回调中处理
					SceneManager::GetSingleton().AddMeshRenderer(meshRenderer);
                }
                else
                {
                    assert(0 && "Not support only vertex mesh.");
                }
                
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

