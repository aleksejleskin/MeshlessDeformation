#include "Input.h"

Input::Input(DirectInput* dI) : m_shutdown(false), m_playerCount(4), m_directInput(dI),
	m_debug(false), m_quit(false)
{
	//initalise all the vectors
	for(unsigned int i = 0; i < (unsigned int)m_playerCount+1; i++)
	{
		vector<bool> newKeys;

		m_controllerInput.push_back(true);
		m_gamepad.push_back(new GamepadInput(i));
	
		for(unsigned int j = 0; j < 4; j++)
		{
			newKeys.push_back(false);
		}

		m_keysPressed.push_back(newKeys);
	}
}

Input::~Input()
{
	if(!m_shutdown)
	{
		Shutdown();
	}
}

void Input::CheckInput()
{
	m_directInput->DetectInput();

	//check all the players input
	for(unsigned int p = 0; p < (unsigned int)m_playerCount; p++)
	{
		//check to see if the gamepad is connected
		if(m_gamepad[p]->IsConnected())
		{
			m_controllerInput[p] = true;
		}

		if(m_controllerInput[p])
		{
			//check all the controller inputs
			CheckGamepadInput(p);
		}
		else
		{
			//check all the keyboard inputs
			CheckKeyboardInput(p);
		}
	}

	if((m_directInput->GetKeyboardState(DIK_F1)) &&
		(!m_directInput->GetKeyboardPrevState(DIK_F1)))
	{
		m_debug = !m_debug;
	}
	if(m_directInput->GetKeyboardState(DIK_ESCAPE))
	{
		m_quit = true;
	}
}

vector<bool> Input::GetKeysPressed(int playerNumber)
{
	//returns the keys that have been pressed for that player
	return m_keysPressed[playerNumber];
}

void Input::SetController(int playerNumber, bool usingController)
{
	//set if the player is using controller or not
	m_controllerInput[playerNumber] = usingController;
}

void Input::Shutdown()
{
	if(!m_shutdown)
	{
		DeleteVector(m_gamepad);
		
		m_shutdown = true;
	}
}

void Input::CheckGamepadInput(int player)
{
	//check if the player controller is connected
	if(m_gamepad[player]->IsConnected())
	{

		//Check to see if the player is pressing A
		if(m_gamepad[player]->GetState().Gamepad.wButtons ==
			XINPUT_GAMEPAD_A)
		{
			m_keysPressed[player][MIDDLE] = true;
		}
		else
		{
			m_keysPressed[player][MIDDLE] = false;
		}

		//Check if the player is pressing the left trigger
		if(m_gamepad[player]->GetState().Gamepad.bLeftTrigger > 5)
		{
			m_keysPressed[player][LEFT] = true;
		}
		else
		{
			m_keysPressed[player][LEFT] = false;
		}

		//Check if the player is pressing the right trigger
		if(m_gamepad[player]->GetState().Gamepad.bRightTrigger > 5)
		{
			m_keysPressed[player][RIGHT] = true;
		}
		else
		{
			m_keysPressed[player][RIGHT] = false;
		}

		//Check if the player is pressing the back button
		if(m_gamepad[player]->GetState().Gamepad.wButtons ==
			XINPUT_GAMEPAD_BACK)
		{
			m_keysPressed[player][BACK] = true;
		}
		else
		{
			m_keysPressed[player][BACK] = false;
		}
	}
	else
	{
		m_controllerInput[player] = false;
	}
}

void Input::CheckKeyboardInput(int player)
{
	int keys[4];

	//Switchs the keys to check dependant on the player
	switch(player)
	{
	case 0:
		keys[0] = DIK_Q;
		keys[1] = DIK_W;
		keys[2] = DIK_E;
		break;
	case 1:
		keys[0] = DIK_Z;
		keys[1] = DIK_X;
		keys[2] = DIK_C;
		break;
	case 2:
		keys[0] = DIK_M;
		keys[1] = DIK_COMMA;
		keys[2] = DIK_PERIOD;
		break;
	case 3:
		keys[0] = DIK_P;
		keys[1] = DIK_LBRACKET;
		keys[2] = DIK_RBRACKET;
		break;
	}

	//Check to see if the left key is pressed
	if(m_directInput->GetKeyboardState(keys[LEFT]))
	{
		m_keysPressed[player][LEFT] = true;
	}
	else
	{
		m_keysPressed[player][LEFT] = false;
	}

	//check to see if the middle key is pressed
	if(m_directInput->GetKeyboardState(keys[MIDDLE]))
	{
		m_keysPressed[player][MIDDLE] = true;
	}
	else
	{
		m_keysPressed[player][MIDDLE] = false;
	}

	//check to see if the right key is pressed
	if(m_directInput->GetKeyboardState(keys[RIGHT]))
	{
		m_keysPressed[player][RIGHT] = true;
	}
	else
	{
		m_keysPressed[player][RIGHT] = false;
	}
}

bool Input::GetDebug() const
{
	return m_debug;
}
float Input::GetMouseX() const
{
	return m_directInput->GetMouseX();
}
float Input::GetMouseY() const
{
	return m_directInput->GetMouseY();
}
bool Input::GetQuit() const
{
	return m_quit;
}
DirectInput* Input::GetDirectInput()
{
	return m_directInput;
}