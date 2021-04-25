#include "pch.h"
#include "Engine.h"
#include "Input.h"

using namespace std;
using namespace DirectX;
using Microsoft::WRL::ComPtr;

Engine* Engine::m_Engine = nullptr;
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    return Engine::Get()->MsgProc(hWnd, message, wParam, lParam);
}

Engine::Engine(HINSTANCE hInstance)
{
    assert(m_Engine == nullptr);
	m_AppInst = hInstance;
	m_Engine = this;
}

Engine::~Engine()
{
    // TODO: 释放GPU资源
    m_CommandContextManager = nullptr;

    m_RenderDevice->PurgeReleaseQueue(true);

    RHI::CommandListManager::GetSingleton().IdleGPU();
}

void Engine::Initialize(const EngineCreateInfo& engineCI)
{
	m_Width = engineCI.Width;
    m_Height = engineCI.Height;
    m_Aspect = static_cast<float>(m_Width) / static_cast<float>(m_Height);
    InitialMainWindow();

	// 开启调试
#if defined(DEBUG) || defined(_DEBUG) 
	{
		ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif
    ID3D12Device* D3D12Device;
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_DXGIFactory)));
	ThrowIfFailed(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&D3D12Device)));

    m_RenderDevice = make_unique<RHI::RenderDevice>(D3D12Device);
    m_CommandListManager = make_unique<RHI::CommandListManager>(D3D12Device);
    m_CommandContextManager = make_unique<RHI::ContextManager>();

    // SwapChaind要在后面创建，因为构造函数中会分配Descriptor，这需要RenderDevice初始化Descriptor Heap，
    // SwapChadin和Graphic Queue关联
    m_SwapChain = make_unique<RHI::SwapChain>(m_Width, m_Height, m_MainWnd, m_DXGIFactory.Get(), 
        RHI::CommandListManager::GetSingleton().GetGraphicsQueue().GetD3D12CommandQueue());

    // Initilize Managers
    m_SceneManager = make_unique<SceneManager>();
    m_ResourceManager = make_unique<ResourceManager>();
    m_vertexFactory = make_unique<VertexFactory>();

	m_Initialized = true;

    m_ForwardRenderer = std::make_unique<ForwardRenderer>();
    m_ForwardRenderer->Initialize();
}

void Engine::Tick()
{
    if (!m_Paused)
    {
	    Update(m_Timer.DeltaTime());
	    Render();
    }
}

void Engine::Update(float deltaTime)
{
    CalculateFrameStats();

    m_SceneManager->UpdateCameraMovement(deltaTime);
    
    Input::Update();
}

void Engine::Render()
{
    m_ForwardRenderer->Render(*m_SwapChain);

    m_SwapChain->Present();
}

LRESULT Engine::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (m_Initialized)
    {
        switch (msg)
        {
        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE)
            {
                Pause();
            }
            else
            {
                Resume();
            }
            return 0;

        case WM_SIZE:
            SetScreenSize(LOWORD(lParam), HIWORD(lParam));
            if (wParam == SIZE_MINIMIZED)
            {
                Pause();
                m_minimized = true;
                m_maximized = false;
            }
            else if (wParam == SIZE_MAXIMIZED)
            {
                Resume();
                m_minimized = false;
                m_maximized = true;
                OnResize();
            }
            else if (wParam == SIZE_RESTORED)
            {

                if (m_minimized)
                {
                    Resume();
                    m_minimized = false;
                    OnResize();
                }

                else if (m_maximized)
                {
                    Resume();
                    m_maximized = false;
                    OnResize();
                }
                else if (m_resizing)
                {
                }
                else 
                {
                    OnResize();
                }
            }
            return 0;

        case WM_ENTERSIZEMOVE:
            Pause();
            m_resizing = true;
            return 0;

        case WM_EXITSIZEMOVE:
            Resume();
            m_resizing = false;
            OnResize();
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_MENUCHAR:
            return MAKELRESULT(0, MNC_CLOSE);

        case WM_GETMINMAXINFO:
            ((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
            ((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
            return 0;

        case WM_LBUTTONDOWN:
            Input::m_KeyStates[(int)KeyCode::Mouse0] = KeyState::KeyDown;
            return 0;

        case WM_MBUTTONDOWN:
            Input::m_KeyStates[(int)KeyCode::Mouse1] = KeyState::KeyDown;
            return 0;

        case WM_RBUTTONDOWN:
            Input::m_KeyStates[(int)KeyCode::Mouse2] = KeyState::KeyDown;
            return 0;

        case WM_LBUTTONUP:
            Input::m_KeyStates[(int)KeyCode::Mouse0] = KeyState::KeyUp;
            return 0;

        case WM_MBUTTONUP:
            Input::m_KeyStates[(int)KeyCode::Mouse1] = KeyState::KeyUp;
            return 0;

        case WM_RBUTTONUP:
            Input::m_KeyStates[(int)KeyCode::Mouse2] = KeyState::KeyUp;
            return 0;

        case WM_MOUSEMOVE:
            Input::OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;

        case WM_KEYDOWN:
            Input::OnKeyDown(wParam);
            return 0;

        case WM_KEYUP:
            if (wParam == VK_ESCAPE)
            {
                PostQuitMessage(0);
            }
            Input::OnKeyUp(wParam);
            return 0;
        }
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void Engine::Destroy()
{
}

void Engine::Pause()
{
    m_Paused = true;
    m_Timer.Stop();
}

void Engine::Resume()
{
    m_Paused = false;
    m_Timer.Start();
}

void Engine::InitialMainWindow()
{
    WNDCLASS wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = m_AppInst;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = L"MainWnd";

    if (!RegisterClass(&wc))
    {
        MessageBox(0, L"RegisterClass Failed.", 0, 0);
    }

    RECT R = { 0, 0, m_Width, m_Height };
    AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
    int width = R.right - R.left;
    int height = R.bottom - R.top;

    wstring m_Title(L"PanGu");
    m_MainWnd = CreateWindow(L"MainWnd", m_Title.c_str(),
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, m_AppInst, 0);
    if (!m_MainWnd)
    {
        MessageBox(0, L"CreateWindow Failed.", 0, 0);
    }

    ShowWindow(m_MainWnd, SW_SHOW);
    UpdateWindow(m_MainWnd);
}

void Engine::SetScreenSize(UINT width, UINT height)
{
    m_Width = width;
    m_Height = height;
    m_Aspect = static_cast<float>(width) / static_cast<float>(height);
}

void Engine::CalculateFrameStats()
{
    static int frameCnt = 0;
    static float timeElapsed = 0.0f;

    frameCnt++;

    // Compute averages over one second period.
    if ((m_Timer.TotalTime() - timeElapsed) >= 1.0f)
    {
        float fps = (float)frameCnt; // fps = frameCnt / 1
        float mspf = 1000.0f / fps;

        wstring fpsStr = to_wstring(fps);
        wstring mspfStr = to_wstring(mspf);

        wstring m_Title(L"PanGu");
        wstring windowText = m_Title +
            L"    fps: " + fpsStr +
            L"   mspf: " + mspfStr;

        SetWindowText(m_MainWnd, windowText.c_str());

        // Reset for next average.
        frameCnt = 0;
        timeElapsed += 1.0f;
    }
}

void Engine::OnResize()
{
    m_SwapChain->Resize(m_Width, m_Height);
}
