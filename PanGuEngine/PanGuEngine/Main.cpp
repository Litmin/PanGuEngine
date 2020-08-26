#include "pch.h"
#include "Engine.h"
#include "RenderCore/GameObject.h"
#include "RenderCore/GeometryFactory.h"
#include "RenderCore/Mesh.h"
#include "RenderCore/StandardShader.h"

using namespace std;
using namespace DirectX;

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	Engine engine;

	try
	{
		engine.Initialize(720, 720, hInstance);

		// TODO:
		GraphicContext::GetSingleton().ResetCommandList();


		// Setup Scene
		// Create Camera
		GameObject* rootNode = SceneManager::GetSingleton().GetRootNode();
		GameObject* cameraNode = rootNode->CreateChild();
		Camera* camera = cameraNode->AddComponent<Camera>();

		GameObject* boxNode = rootNode->CreateChild();
		boxNode->Translate(0.0f, 1.0f, 0.0f, Space::World);
		//// Mesh
		UINT boxVertexCount, boxIndicesCount;
		vector<XMFLOAT3> boxPositions;
		vector<XMFLOAT4> boxColors;
		vector<uint16_t> boxIndices;
		GeometryFactory::CreateBox(1.0f, 1.0f, 1.0f, 1, boxVertexCount, boxPositions, boxColors, boxIndicesCount, boxIndices);
		unique_ptr<Mesh> boxMesh = make_unique<Mesh>(boxVertexCount, boxPositions.data(), boxColors.data(),
			nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
			boxIndicesCount, boxIndices.data());
		// Shader
		unique_ptr<StandardShader> standardShader = make_unique<StandardShader>();
		standardShader->Initialize(GraphicContext::GetSingleton().Device());
		// Material
		unique_ptr<Material> material = make_unique<Material>(standardShader.get());
		// MeshRenderer
		MeshRenderer* meshRenderer = boxNode->AddComponent<MeshRenderer>();
		meshRenderer->SetMesh(boxMesh.get());
		meshRenderer->SetMaterial(material.get());

		SceneManager::GetSingleton().AddCamera(camera);
		SceneManager::GetSingleton().AddMeshRenderer(meshRenderer);


		GraphicContext::GetSingleton().BuildFrameResource();
		SceneManager::GetSingleton().BuildConstantBuffer();

		GraphicContext::GetSingleton().ExecuteCommandList();
		engine.FlushCommandQueue();

		return engine.Run();
		// TODO: Clean
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}

}

