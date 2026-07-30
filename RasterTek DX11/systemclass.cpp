#include "systemclass.h"

SystemClass::SystemClass()
{
    m_Input = 0;
    m_Application = 0;
}


SystemClass::SystemClass(const SystemClass& other)
{
    
}

SystemClass::~SystemClass()
{
    
}

bool SystemClass::Initialize()
{
    int screenWidth, screenHeight;
    bool result;
    
    screenWidth = 0;
    screenHeight = 0;
    
    InitializeWindows(screenWidth, screenHeight);
    
    m_Input = new InputClass;
    m_Input->Initialize();
    
    m_Application = new ApplicationClass;
    
    result = m_Application->Initialize(screenWidth, screenHeight, m_hwnd);
    if (!result)
    {
        return false;
    }
    return true;
}

void SystemClass::Shutdown()
{
    if (m_Application)
    {
        m_Application->Shutdown();
        delete m_Application;
        m_Application = 0;
    }
    
    if (m_Input)
    {
        delete m_Input;
        m_Input = 0;
    }
    ShutdownWindows();
    return;
}

void SystemClass::Run()
{
    MSG msg;
    bool done, result;
    
    done = false;
    while (!done)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        if (msg.message == WM_QUIT)
        {
            done = true;
        }
        else
        {
            result = Frame();
            if (!result)
            {
                done = true;
            }
        }
    }
    return;
}

bool SystemClass::Frame()
{
    bool result;
    
    // ESC키로 종료하는지 확인
    if (m_Input->IsKeyDown(VK_ESCAPE))
    {
        return false;
    }
    result = m_Application->Frame();
    if (!result)
    {
        return false;
    }
    return true;
}

LRESULT CALLBACK SystemClass::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
    switch (umsg)
    {
        // 키가 눌렸는지, 눌렸으면 입력객체로 보내 그 상태를 기록
        case WM_KEYDOWN:
        {
            m_Input->KeyDown((unsigned int)wparam);
            return 0;    
        }
        // 키가 떼어졌는지, 떼어졌으면 입력객체로 보내 그 상태를 해제하게
        case WM_KEYUP:
        {
            m_Input->KeyUp((unsigned int)wparam);
            return 0;    
        }
        // 그 외 메시지는 애플리케이션에서 쓰지 않으므로 기본 메시지 핸들러로
        default:
        {
            return DefWindowProc(hwnd, umsg, wparam, lparam);
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
    switch(umessage)
    {
        // 창이 파괴되는지 확인
        case WM_DESTROY:
            {
                PostQuitMessage(0);
                return 0;
            }
        // 창이 닫히는지 확인
        case WM_CLOSE:
            {
                PostQuitMessage(0);
                return 0;
            }
        default:
            {
                return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
            }
    }
}

void SystemClass::InitializeWindows(int& screenWidth, int& screenHeight)
{
    WNDCLASSEX wc;
    DEVMODE dmScreenSettings;
    int posX, posY;
    
    ApplicationHandle = this;
    m_hinstance = GetModuleHandle(NULL);
    m_applicationName = L"Engine";
    
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = m_hinstance;
    wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    wc.hIconSm = wc.hIcon;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = m_applicationName;
    wc.cbSize = sizeof(WNDCLASSEX);
    
    RegisterClassEx(&wc);
    
    screenWidth = GetSystemMetrics(SM_CXSCREEN);
    screenHeight = GetSystemMetrics(SM_CYSCREEN);
    
    if (FULL_SCREEN)
    {
        memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
        dmScreenSettings.dmSize = sizeof(dmScreenSettings);
        dmScreenSettings.dmPelsWidth = (unsigned long)screenWidth;
        dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
        dmScreenSettings.dmBitsPerPel = 32;
        dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

        // 디스플레이 설정을 전체화면으로 변경한다.
        ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

        // 창 위치를 좌측 상단 모서리로 설정한다.
        posX = posY = 0;
    } else
    {
        screenWidth = 800;
        screenHeight = 600;
        
        posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
        posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
    }
    
    m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, 
        m_applicationName, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
                            posX, posY, screenWidth, screenHeight, NULL, 
                            NULL, m_hinstance, NULL);
    
    ShowWindow(m_hwnd, SW_SHOW);
    SetForegroundWindow(m_hwnd);
    SetFocus(m_hwnd);
    
    ShowCursor(false);
    return;
}

void SystemClass::ShutdownWindows()
{
    ShowCursor(true);
    if (FULL_SCREEN)
    {
        ChangeDisplaySettings(NULL, 0);
    }
    DestroyWindow(m_hwnd);
    m_hwnd = NULL;
    
    UnregisterClass(m_applicationName, m_hinstance);
    m_hinstance = NULL;
    
    ApplicationHandle = NULL;
    return;
}