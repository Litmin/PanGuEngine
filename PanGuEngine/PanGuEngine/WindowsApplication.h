#pragma once

#include "PanGuEngine.h"


class WindowsApplication
{
public:
    WindowsApplication(HINSTANCE hInstance, PanGuEngine* engine);
    ~WindowsApplication();

public:
    static WindowsApplication* GetApp();

    bool Initialize(UINT width, UINT height);
    int Run();
    HWND GetHwnd() { return m_hwnd; }
    LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


private:
    static WindowsApplication* m_app;

    HINSTANCE m_appInst;
    HWND m_hwnd;
    PanGuEngine* m_engine;


    bool m_resizing = false;
    bool m_minimized = false;
    bool m_maximized = false;
};

