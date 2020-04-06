#include "pch.h"
#include "WindowsApplication.h"

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return WindowsApplication::GetApp()->MsgProc(hWnd, message, wParam, lParam);
}

WindowsApplication::WindowsApplication(HINSTANCE hInstance, PanGuEngine* engine) :
	m_appInst(hInstance),
	m_engine(engine),
	m_hwnd(nullptr)
{
	assert(m_app == nullptr);
	m_app = this;
}

WindowsApplication::~WindowsApplication()
{
}

WindowsApplication* WindowsApplication::m_app = nullptr;
WindowsApplication* WindowsApplication::GetApp()
{
	return m_app;
}

bool WindowsApplication::Initialize(UINT width, UINT height)
{
	// 命令行参数
	int argc;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	LocalFree(argv);

	// 创建Windows窗口
	WNDCLASSEX windowsClass = { 0 };
	windowsClass.cbSize = sizeof(WNDCLASSEX);
	windowsClass.style = CS_HREDRAW | CS_VREDRAW;
	windowsClass.lpfnWndProc = WindowProc;
	windowsClass.hInstance = m_appInst;
	windowsClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowsClass.lpszClassName = L"PanGu";

	RegisterClassEx(&windowsClass);

	RECT windowRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	m_hwnd = CreateWindow(
		windowsClass.lpszClassName,
		L"PanGu",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr,        // We have no parent window.
		nullptr,        // We aren't using menus.
		m_appInst,
		nullptr);

	ShowWindow(m_hwnd, SW_SHOW);

	return false;
}

int WindowsApplication::Run()
{
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		// Process any messages in the queue.
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		m_engine->Tick();
	}

	m_engine->Destroy();

	return (int)msg.wParam;
}

LRESULT WindowsApplication::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		// WM_ACTIVATE is sent when the window is activated or deactivated.  
		// We pause the game when the window is deactivated and unpause it 
		// when it becomes active.  
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			// Pause
			m_engine->Pause();
		}
		else
		{
			// Resume
			m_engine->Resume();
		}
		return 0;

		// WM_SIZE is sent when the user resizes the window.  
	case WM_SIZE:
		// Save the new client area dimensions.
		m_engine->SetScreenSize(LOWORD(lParam), HIWORD(lParam));
		if (wParam == SIZE_MINIMIZED)
		{
			m_engine->Pause();
			m_minimized = true;
			m_maximized = false;
		}
		else if (wParam == SIZE_MAXIMIZED)
		{
			m_engine->Resume();
			m_minimized = false;
			m_maximized = true;
			m_engine->OnResize();
		}
		else if (wParam == SIZE_RESTORED)
		{

			// Restoring from minimized state?
			if (m_minimized)
			{
				m_engine->Resume();
				m_minimized = false;
				m_engine->OnResize();
			}

			// Restoring from maximized state?
			else if (m_maximized)
			{
				m_engine->Resume();
				m_maximized = false;
				m_engine->OnResize();
			}
			else if (m_resizing)
			{
				// If user is dragging the resize bars, we do not resize 
				// the buffers here because as the user continuously 
				// drags the resize bars, a stream of WM_SIZE messages are
				// sent to the window, and it would be pointless (and slow)
				// to resize for each WM_SIZE message received from dragging
				// the resize bars.  So instead, we reset after the user is 
				// done resizing the window and releases the resize bars, which 
				// sends a WM_EXITSIZEMOVE message.
			}
			else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
			{
				m_engine->OnResize();
			}
		}
		return 0;

		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		m_engine->Pause();
		m_resizing = true;
		return 0;

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		m_engine->Resume();
		m_resizing = false;
		m_engine->OnResize();
		return 0;

		// WM_DESTROY is sent when the window is being destroyed.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		// The WM_MENUCHAR message is sent when a menu is active and the user presses 
		// a key that does not correspond to any mnemonic or accelerator key. 
	case WM_MENUCHAR:
		// Don't beep when we alt-enter.
		return MAKELRESULT(0, MNC_CLOSE);

		// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	/*case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_KEYUP:
		if (wParam == VK_ESCAPE)
		{
			PostQuitMessage(0);
		}
		else if ((int)wParam == VK_F2)
			Set4xMsaaState(!m4xMsaaState);

		return 0;*/
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

