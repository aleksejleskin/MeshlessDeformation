/*
	Contains the handler for all the diffrent libarys used:
	D3D
	DInput
	Game Timer
*/

#ifndef SYSTEM_H
#define SYSTEM_H

#include <iostream>
#include <string>
#include <Windows.h>
#include "DxGraphics.h"
#include "Timer.h"
#include "DirectInput.h"
#include <AntTweakBar.h>

using std::string;

LRESULT CALLBACK ForwardProc(HWND hwnd, UINT message, WPARAM wParam,
	LPARAM lParam);

class System
{
public:
	System(int windowWidth, int windowHeight);
	~System();

	bool Initialise(HINSTANCE hInstance, HINSTANCE pInstance,
		LPWSTR cmdLine, int cmdShow);
	LRESULT WndProc(HWND hwnd, UINT message, WPARAM wParam,
		LPARAM lParam);
	void Shutdown();
	bool Done();

	HINSTANCE GetHinstance();
	HWND GetHwnd();
	MSG GetMsg();
	int GetWindowWidth() const;
	int GetWindowHeight() const;
	float GetAspectRatio() const;

	DxGraphics* GetDX();
	Timer* GetGameTimer();
	DirectInput* GetDirectInput();


private:
	bool InitialiseWindow(HINSTANCE hInstance, HINSTANCE pInstance,
		LPWSTR cmdLine, int cmdShow);

	HWND m_hwnd;
	HINSTANCE m_hInstance;
	MSG m_msg;
	int m_windowWidth,
		m_windowHeight;

	bool m_shutdown,
		m_resizing;

	string m_windowName;
	DxGraphics m_dxGraphics;
	Timer m_gameTimer;
	DirectInput m_directInput;
	
};

#endif