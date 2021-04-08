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
		Engine engine;
		EngineCreateInfo engineCI;
		engineCI.Width = 1920;
		engineCI.Height = 1080;

		engine.Initialize(1920, 1080, hInstance);

		Resource::GLTFLoader::LoadGLTF("Resources/DamagedHelmet.gltf", SceneManager::GetSingleton().GetRootNode());

		// Create Camera
		GameObject* rootGo = SceneManager::GetSingleton().GetRootNode();
		GameObject* cameraGo = rootGo->CreateChild();
		Camera* camera = cameraGo->AddComponent<Camera>();
		cameraGo->Translate(0.0f, 0.0f, -5.0f, Space::Self);


		//// Mesh
		//UINT boxVertexCount, boxIndicesCount;
		//std::vector<XMFLOAT3> boxPositions;
		//std::vector<XMFLOAT4> boxColors;
		//std::vector<XMFLOAT3> normals;
		//std::vector<XMFLOAT4> tangents;
		//std::vector<XMFLOAT2> uvs;
		//std::vector<UINT32> boxIndices;
		//GeometryFactory::CreateBox(1.0f, 1.0f, 1.0f, 0, boxVertexCount, boxPositions, boxColors, normals, tangents, uvs, boxIndicesCount, boxIndices);
		//std::shared_ptr<Mesh> boxMesh = std::make_shared<Mesh>(boxVertexCount, (const float*)boxPositions.data(), nullptr,
		//	(const float*)normals.data(), nullptr, (const float*)uvs.data(), nullptr, nullptr, nullptr,
		//	boxIndicesCount, boxIndices.data());

		//GameObject* boxGo = rootGo->CreateChild();
		//boxGo->Translate(0.0f, -10.0f, 0.0f);
		//MeshRenderer* meshRenderer = boxGo->AddComponent<MeshRenderer>();
		//meshRenderer->SetMesh(boxMesh);

		return engine.Run();
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}

}

