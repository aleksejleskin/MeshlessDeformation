#include "System.h"

namespace
{
	System* tmpSystem = 0;
}

//Call the windows procedure before the initalise has been called
// this needs to be done so we can have the window proc private
LRESULT CALLBACK ForwardProc(HWND hwnd, UINT message, WPARAM wParam,
	LPARAM lParam)
{
	return tmpSystem->WndProc(hwnd, message, wParam, lParam);
}

System::System(int windowWidth, int windowHeight) :
m_windowWidth(windowWidth), m_windowHeight(windowHeight),
	m_hwnd(0), m_hInstance(0), m_shutdown(false),
	m_windowName("Project: GreenLight"), m_resizing(false)
{
	//assign the tmpSystem which called the forward Prox
	//to this instance
	tmpSystem = this;
}

System::~System()
{
	if(!m_shutdown)
	{
		Shutdown();
	}
}

bool System::Initialise(HINSTANCE hInstance, HINSTANCE pInstance,
		LPWSTR cmdLine, int cmdShow)
{
	//Initalise the window
	if(!InitialiseWindow(hInstance, pInstance, cmdLine, cmdShow))
	{
		MessageBoxA(NULL, "Failed to initialise the window", "ERROR", MB_OK | MB_ICONERROR);
		return false;
	}

	//Initalise D3D
	if(!m_dxGraphics.InitialiseDX(m_hwnd,m_windowWidth, m_windowHeight))
	{
		MessageBoxA(NULL, "Failed to initialise D3D", "ERROR", MB_OK | MB_ICONERROR);
		return false;
	}

	//Initalise DInput
	if(!m_directInput.InitialiseDirectInput(m_hInstance, m_hwnd))
	{
		MessageBoxA(NULL, "Failed to initialise DInput", "ERROR", MB_OK | MB_ICONERROR);
		return false;
	}

	return true;
}

//Main window procedure
LRESULT System::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	if (TwEventWin(hwnd, message, wParam, lParam))
		return 0; // Event has been handled by AntTweakBar
	PAINTSTRUCT paintStruct;
	HDC hdc;

	switch(message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &paintStruct);
		EndPaint(hwnd, &paintStruct);
		break;
	case WM_SIZE:
		//RESIZE BUFFERS WHEN WINDOW CHANGED
		m_windowWidth = LOWORD(lParam);
		m_dxGraphics.SetWindowWidth(m_windowWidth);
		m_windowHeight = HIWORD(lParam);
		m_dxGraphics.SetWindowHeight(m_windowHeight);

		if(m_dxGraphics.GetDevice())
		{
			if( wParam == SIZE_MAXIMIZED)
			{
				m_dxGraphics.Rebuild(hwnd);
			}
			if(!m_resizing)
			{
				m_dxGraphics.Rebuild(hwnd);
			}
		}
		break;
	case WM_ENTERSIZEMOVE:
		//check to see if the user is resizing the window
		m_resizing = true;
		break;
	case WM_EXITSIZEMOVE:
		//check to see if the user has stopped resizing the window
		m_resizing = false;
		m_dxGraphics.Rebuild(hwnd);
		break;
	case WM_MENUCHAR:
		//stops beeping when we alt enter to go fullscreen
		return MAKELRESULT(0, MNC_CLOSE);
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
		break;
	}

	return 0;
}

void System::Shutdown()
{
	if(!m_shutdown)
	{
		m_dxGraphics.Shutdown();
		m_shutdown = true;
	}
}

bool System::Done()
{
	//If the message = quit then exit the game loop
	if(m_msg.message == WM_QUIT)
	{
		return true;
	}

	//Check the next message and process it
	if(PeekMessage(&m_msg, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&m_msg);
		DispatchMessage(&m_msg);
	}

	return false;
}

bool System::InitialiseWindow(HINSTANCE hInstance, HINSTANCE pInstance,
		LPWSTR cmdLine, int cmdShow)
{
	m_hInstance = hInstance;

	UNREFERENCED_PARAMETER(cmdLine);
	UNREFERENCED_PARAMETER(pInstance);

	//Set up the window using this class desc
	WNDCLASSEX wnd = { 0 };
	wnd.cbSize = sizeof(WNDCLASSEX);
	wnd.style = CS_HREDRAW | CS_VREDRAW;
	wnd.lpfnWndProc = ForwardProc;
	wnd.hInstance = m_hInstance;
	wnd.hCursor = LoadCursor(NULL, IDC_ARROW);
	wnd.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wnd.lpszMenuName = NULL;
	wnd.lpszClassName = "wndClass";

	//If the class fails to register then return false, (in Debug will also throw a messagebox error)
	if(!RegisterClassEx(&wnd))
	{
#if defined (DEBUG) || (_DEBUG)
		MessageBox(NULL, "The window class failed to register!", "ERROR!", MB_OK | MB_ICONERROR);
#endif
		return false;
	}

	//Describe the window dimensions using the RECT struct
	RECT rc = {0, 0, m_windowWidth, m_windowHeight};

	//Creates the window using the previous class we made
	m_hwnd = CreateWindowA("wndClass", m_windowName.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
		CW_USEDEFAULT, (rc.right - rc.left), (rc.bottom - rc.top) , NULL, NULL, m_hInstance, NULL);

	//If the window failed to create return false (in Debug will also throw a messagebox error)
	if(!m_hwnd)
	{
#if defined (DEBUG) || (_DEBUG)
		MessageBox(NULL, "The window failed to create", "ERROR!", MB_OK | MB_ICONERROR);
#endif
		return false;
	}

	//Display the window
	ShowWindow(m_hwnd, cmdShow);

	return true;
}

HINSTANCE System::GetHinstance()
{
	return m_hInstance;
}
HWND System::GetHwnd()
{
	return m_hwnd;
}
MSG System::GetMsg()
{
	return m_msg;
}
int System::GetWindowWidth() const
{
	return m_windowWidth;
}
int System::GetWindowHeight() const
{
	return m_windowHeight;
}
float System::GetAspectRatio() const
{
	return (float) m_windowWidth / m_windowHeight;
}

DxGraphics* System::GetDX()
{
	return &m_dxGraphics;
}

Timer* System::GetGameTimer()
{
	return &m_gameTimer;
}

DirectInput* System::GetDirectInput()
{
	return &m_directInput;
}