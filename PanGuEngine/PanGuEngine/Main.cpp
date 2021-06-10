#include "pch.h"
#include "Engine.h"
#include "Renderer/GameObject.h"
#include "Utility/GeometryFactory.h"
#include "Renderer/Mesh.h"
#include "Resource/GLTFLoader.h"

using namespace DirectX;

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	try
	{
		Engine engine(hInstance);

		EngineCreateInfo engineCI;
		engineCI.Width = 1920;
		engineCI.Height = 1080;

		auto setup = []()
		{
			GameObject* rootNode = SceneManager::GetSingleton().GetRootNode();

			GameObject* damagedHelmet = Resource::GLTFLoader::LoadGLTF("Resources/DamagedHelmet.gltf", rootNode);
			damagedHelmet->Rotate(90.0f, 180.0f, 0.0f);
			//damagedHelmet->Rotate(0, 180, 0, Space::Self);
			//Resource::GLTFLoader::LoadGLTF("Resources/SciFiHelmet/SciFiHelmet.gltf", rootNode);
			//Resource::GLTFLoader::LoadGLTF("Resources/Sponza/Sponza.gltf", rootNode);
			/*GameObject* fish = Resource::GLTFLoader::LoadGLTF("Resources/BarramundiFish.glb", rootNode);
			fish->Translate(0.0f, 0.8f, 0.0f, Space::Self);
			fish->Rotate(90.0f, 0.0f, 0.0f);
			fish->SetLocalScale(Math::Vector3(10.0f, 10.0f, 10.0f));*/

			GameObject* cameraGo = rootNode->CreateChild();
			Camera* camera = cameraGo->AddComponent<Camera>();
			camera->SetProjection(1920.0f / 1080.0f, 0.1f, 100.0f, MathHelper::Pi / 3.0f);
			cameraGo->Translate(0.0f, 0.0f, -5.0f, Space::Self);

			GameObject* lightGo = rootNode->CreateChild();
			Light* light = lightGo->AddComponent<Light>();
			light->SetLightColor(DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f));
			light->SetLightIntensity(1.0f);
			lightGo->Rotate(45.0f, 0.0f, 0.0f, Space::Self);

			// Plane
			std::shared_ptr<Mesh> boxMesh = GeometryFactory::CreateBox(1.0f, 1.0f, 1.0f, 0);
			GameObject* boxGo = rootNode->CreateChild();
			boxGo->Translate(0.0f, -3.0f, 0.0f);
			boxGo->SetLocalScale(Math::Vector3(30.0f, 1.0f, 30.0f));
			MeshRenderer* meshRenderer = boxGo->AddComponent<MeshRenderer>();
			meshRenderer->SetMesh(boxMesh);
			DirectX::XMFLOAT4 oneVector = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			std::shared_ptr<Material> material = std::make_shared<Material>(0.5f, 0.5f, oneVector, oneVector, ResourceManager::GetSingleton().GetDefaultWhiteTex(),
				nullptr, nullptr, nullptr, ResourceManager::GetSingleton().GetDefaultBlackTex());
			meshRenderer->SetMaterial(material);
		};

		return engine.RunN(engineCI, setup);
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}

}

