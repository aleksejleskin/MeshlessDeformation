#include "DirectInput.h"

DirectInput::DirectInput() : m_keyboard(0), m_directInput(0), m_mouse(0)
{
}

DirectInput::~DirectInput()
{
}

bool DirectInput::InitialiseDirectInput(HINSTANCE hInstance, HWND hwnd)
{
	//Creates the direct input device
	HRESULT hr = DirectInput8Create(hInstance,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&m_directInput,
		NULL);

	if(FAILED(hr))
	{
		return false;
	}

	//Creates our keyboard object device
	hr = m_directInput->CreateDevice(GUID_SysKeyboard, &m_keyboard, NULL);

	if(FAILED(hr))
	{
		return false;
	}

	//creates our mouse object device
	hr = m_directInput->CreateDevice(GUID_SysMouse, &m_mouse, NULL);

	if(FAILED(hr))
	{
		return false;
	}

	//tell direct input to expect keyboard inputs of a 256 array of keys
	HR(m_keyboard->SetDataFormat(&c_dfDIKeyboard));
	HR(m_keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE));

	HR(m_mouse->SetDataFormat(&c_dfDIMouse));
	HR(m_mouse->SetCooperativeLevel(hwnd, DISCL_NONEXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND));

	return true;
}

void DirectInput::DetectInput()
{
	//assign the previous state
	m_lastMouseState = m_currentMouseState;
	for(unsigned int i = 0; i < 256; i++)
	{
		m_keyboardPrevState[i] = m_keyboardState[i];
	}

	//Retreive the keybaord & mouse to be used for this application
	m_keyboard->Acquire();
	m_mouse->Acquire();
	
	//Get the device state
	m_keyboard->GetDeviceState(sizeof(m_keyboardState), (LPVOID)&m_keyboardState);
	m_mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&m_currentMouseState);
}

bool DirectInput::GetKeyboardState(int key)
{
	//Check the key passed thorugh to see if its pressed
	if(m_keyboardState[key] & 0x80)
	{
		return true;
	}
	else
	{
		return false;
	}

	return false;
}

bool DirectInput::GetKeyboardPrevState(int key)
{
	//Check the key passed thorugh to see if its pressed
	if(m_keyboardPrevState[key] & 0x80)
	{
		return true;
	}
	else
	{
		return false;
	}

	return false;
}

void DirectInput::Shutdown()
{

	if(m_keyboard)
	{
		m_keyboard->Unacquire();
		m_mouse->Unacquire();
	}

	ReleaseCOM(m_directInput);

}

float DirectInput::GetMouseX() const
{
	return (float)m_currentMouseState.lX;
}
float DirectInput::GetMouseY() const
{
	return (float)m_currentMouseState.lY;
}

