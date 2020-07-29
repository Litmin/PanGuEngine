#include "pch.h"
#include "WindowsApplication.h"
#include "RenderCore/SceneNode.h"

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	Engine engine;
	WindowsApplication application(hInstance, &engine);

	try
	{
		application.Initialize(720, 720);
		engine.Initialize(720, 720, application.GetHwnd());

		// Create Scene
		// Create Camera
		SceneNode* rootNode = SceneManager::GetSingleton().GetRootNode();
		SceneNode* cameraNode = rootNode->CreateChildNode();
		Camera* camera = new Camera();
		cameraNode->AttachObject(camera);
		cameraNode->Translate(0.0f, 0.0f, -10.0f);

		SceneNode* boxNode = rootNode->CreateChildNode();
		// Mesh


		// Material

		// MeshRenderer


		return application.Run();
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}

}

