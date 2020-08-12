#include "pch.h"
#include "WindowsApplication.h"
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
	WindowsApplication application(hInstance, &engine);

	try
	{
		application.Initialize(720, 720);
		engine.Initialize(720, 720, application.GetHwnd());

		// TODO:
		GraphicContext::GetSingleton().ResetCommandList();


		// Setup Scene
		// Create Camera
		GameObject* rootNode = SceneManager::GetSingleton().GetRootNode();
		GameObject* cameraNode = rootNode->CreateChild();
		Camera* camera = new Camera();
		cameraNode->AttachObject(camera);
		cameraNode->Translate(0.0f, 0.0f, -10.0f);

		GameObject* boxNode = rootNode->CreateChild();
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
		MeshRenderer* meshRenderer = new MeshRenderer(boxMesh.get(), material.get());
		boxNode->AttachObject(meshRenderer);
		// TODO:
		meshRenderer->SetGameObject(boxNode);

		GraphicContext::GetSingleton().BuildFrameResource();
		SceneManager::GetSingleton().BuildConstantBuffer();

		GraphicContext::GetSingleton().ExecuteCommandList();
		engine.FlushCommandQueue();

		return application.Run();
		// TODO: Clean
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}

}

