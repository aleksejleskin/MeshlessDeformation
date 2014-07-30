#ifndef DIRECT_INPUT_H
#define DIRECT_INPUT_H

#include <Windows.h>
#include <dinput.h>
#include "Utilities.h"

class DirectInput
{
public:
	DirectInput();
	~DirectInput();

	bool InitialiseDirectInput(HINSTANCE hInstance, HWND hwnd);
	
	void DetectInput();

	bool GetKeyboardState(int key);
	bool GetKeyboardPrevState(int key);

	float GetMouseX() const;
	float GetMouseY() const;

	void Shutdown();
private:

	DIMOUSESTATE m_lastMouseState;
	DIMOUSESTATE m_currentMouseState;

	IDirectInputDevice8* m_keyboard;
	IDirectInputDevice8* m_mouse;

	LPDIRECTINPUT8 m_directInput;
	BYTE m_keyboardState[256];
	BYTE m_keyboardPrevState[256];
};

#endif