#include "pch.h"
#include "Engine.h"
#include "Renderer/GameObject.h"
#include "Renderer/GeometryFactory.h"
#include "Renderer/Mesh.h"
#include "Renderer/StandardShader.h"

using namespace std;
using namespace DirectX;

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	Engine engine;

	try
	{
		engine.Initialize(1920, 1080, hInstance);


		// Setup Scene
		// Create Camera
		GameObject* rootGo = SceneManager::GetSingleton().GetRootNode();
		GameObject* cameraGo = rootGo->CreateChild();
		Camera* camera = cameraGo->AddComponent<Camera>();
		cameraGo->Translate(0.0f, 0.0f, -5.0f, Space::Self);


		//// Mesh
		UINT boxVertexCount, boxIndicesCount;
		vector<XMFLOAT3> boxPositions;
		vector<XMFLOAT4> boxColors;
		vector<XMFLOAT3> normals;
		vector<XMFLOAT4> tangents;
		vector<XMFLOAT2> uvs;
		vector<uint16_t> boxIndices;
		GeometryFactory::CreateBox(1.0f, 1.0f, 1.0f, 0, boxVertexCount, boxPositions, boxColors, normals, tangents, uvs, boxIndicesCount, boxIndices);
		unique_ptr<Mesh> boxMesh = make_unique<Mesh>(boxVertexCount, boxPositions.data(), boxColors.data(),
			normals.data(), tangents.data(), uvs.data(), nullptr, nullptr, nullptr,
			boxIndicesCount, boxIndices.data());

		GameObject* boxGo = rootGo->CreateChild();
		boxGo->Translate(-1.0f, 1.0f, -1.0f);
		MeshRenderer* meshRenderer = boxGo->AddComponent<MeshRenderer>();
		meshRenderer->SetMesh(boxMesh.get());

		GameObject* boxGo2 = rootGo->CreateChild();
		boxGo2->Translate(1.0f, 0.0f, 0.0f);
		MeshRenderer* meshRenderer2 = boxGo2->AddComponent<MeshRenderer>();
		meshRenderer2->SetMesh(boxMesh.get());


		// TODO: 在Component的Add回调中处理
		SceneManager::GetSingleton().AddCamera(camera);
		SceneManager::GetSingleton().AddMeshRenderer(meshRenderer);
		SceneManager::GetSingleton().AddMeshRenderer(meshRenderer2);


		return engine.Run();
		// TODO: Clean
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}

}

