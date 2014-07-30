#ifndef INPUT_H
#define INPUT_H

#include <iostream>
#include <vector>
#include "DirectInput.h"
#include "GamepadInput.h"
#include "Utilities.h"

using std::vector;
using namespace Utilities;

class Input
{
public:
	Input(DirectInput* dI);
	~Input();

	void CheckInput();

	vector<bool> GetKeysPressed(int playerNumber);
	float GetMouseX() const;
	float GetMouseY() const;
	bool GetDebug() const;
	bool GetQuit() const;

	void SetController(int playerNumber, bool usingController);

	void Shutdown();

	DirectInput* GetDirectInput();
private:

	int m_playerCount;
	bool m_shutdown,
		m_debug,
		m_quit;

	vector<bool> m_controllerInput;
	vector<vector<bool>> m_keysPressed;

	DirectInput* m_directInput;

	vector<GamepadInput*> m_gamepad;

	void CheckGamepadInput(int player);
	void CheckKeyboardInput(int player);

	enum m_keyNames {LEFT = 0, MIDDLE = 1, RIGHT = 2, BACK = 3};
};

#endif