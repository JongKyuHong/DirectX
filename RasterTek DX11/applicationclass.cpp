#include "applicationclass.h"


ApplicationClass::ApplicationClass()
{
	m_Direct3D = 0;
	m_Camera = 0;
	m_TextureShader = 0;
	m_Sprite = 0;
	m_Timer = 0;
}


ApplicationClass::ApplicationClass(const ApplicationClass& other)
{
}


ApplicationClass::~ApplicationClass()
{
}


bool ApplicationClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	char spriteFilename[128];
	bool result;

	m_Direct3D = new D3DClass;

	result = m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
		return false;
	}

	m_Camera = new CameraClass;
	m_Camera->SetPosition(0.0f, 0.0f, -10.0f);
	m_Camera->Render();

	m_TextureShader = new TextureShaderClass;

	result = m_TextureShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the texture shader object.", L"Error", MB_OK);
		return false;
	}

	strcpy_s(spriteFilename, "../Engine/data/sprite_data_01.txt");

	// sprite 객체를 생성하고 초기화한다.
	m_Sprite = new SpriteClass;

	result = m_Sprite->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), screenWidth, screenHeight, spriteFilename, 50, 50);
	if(!result)
	{
		return false;
	}

	// timer 객체를 생성하고 초기화한다.
	m_Timer = new TimerClass;

	result = m_Timer->Initialize();
	return true;
}


void ApplicationClass::Shutdown()
{
	if(m_Timer)
	{
		delete m_Timer;
		m_Timer = 0;
	}
	
	if(m_Sprite)
	{
		m_Sprite->Shutdown();
		delete m_Sprite;
		m_Sprite = 0;
	}

	if(m_TextureShader)
	{
		m_TextureShader->Shutdown();
		delete m_TextureShader;
		m_TextureShader = 0;
	}

	if(m_Camera)
	{
		delete m_Camera;
		m_Camera = 0;
	}

	if(m_Direct3D)
	{
		m_Direct3D->Shutdown();
		delete m_Direct3D;
		m_Direct3D = 0;
	}

	return;
}


bool ApplicationClass::Frame()
{
	float frameTime;
	bool result;
	
	// 시스템 통계를 갱신한다.
	m_Timer->Frame();

	// 현재 프레임 시간을 가져온다.
	frameTime = m_Timer->GetTime();

	// 프레임 시간을 사용해 스프라이트 객체를 갱신한다.
	m_Sprite->Update(frameTime);

	result = Render();
	if(!result)
	{
		return false;
	}

	return true;
}


bool ApplicationClass::Render()
{
	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;
	bool result;

	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetOrthoMatrix(orthoMatrix);

	m_Direct3D->TurnZBufferOff();

	result = m_Sprite->Render(m_Direct3D->GetDeviceContext());
	if(!result)
	{
		return false;
	}

	result = m_TextureShader->Render(m_Direct3D->GetDeviceContext(), m_Sprite->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix, m_Sprite->GetTexture());
	if(!result)
	{
		return false;
	}

	m_Direct3D->TurnZBufferOn();
	m_Direct3D->EndScene();
	return true;
}
