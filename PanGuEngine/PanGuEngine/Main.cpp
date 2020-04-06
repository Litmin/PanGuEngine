#include "pch.h"
#include "WindowsApplication.h"

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	PanGuEngine engine;
	WindowsApplication application(hInstance, &engine);

	try
	{
		application.Initialize(720, 720);
		engine.Initialize(720, 720, application.GetHwnd());
		return application.Run();
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}

}

