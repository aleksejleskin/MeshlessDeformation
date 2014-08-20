#include "DebugCamera.h"

DebugCamera::DebugCamera() : Camera(), m_rotate(0.0f), m_aimX(0.0f), m_aimY(0.0f), m_mouseSens(100.0f)
{
}

DebugCamera::~DebugCamera()
{
}

void DebugCamera::Initialise(XMFLOAT3 position, float yaw, float pitch, float roll,
	float fov, float aspectRatio, XMFLOAT3 target)
{
	Camera::Initialise(position, yaw, pitch, roll,
		fov, aspectRatio, target);
}

void DebugCamera::Update(float dt, DirectInput* dInput)
{
	CalculateViewMatrix();
	if (dInput->GetKeyboardState(DIK_NUMPAD8))
	{
		XMVECTOR vMove = GetUpDown();
		vMove *= 100.0f * dt;
		MoveCamera(vMove);
	}

	if (dInput->GetKeyboardState(DIK_NUMPAD5))
	{
		XMVECTOR vMove = -GetUpDown();
		vMove *= 100.0f*dt;
		MoveCamera(vMove);
	}

	if (dInput->GetKeyboardState(DIK_NUMPAD4))
	{
		XMVECTOR vMove = -GetRightLeft();
		vMove *= 100.0f*dt;
		MoveCamera(vMove);
	}

	if (dInput->GetKeyboardState(DIK_NUMPAD6))
	{
		XMVECTOR vMove = GetRightLeft();
		vMove *= 100.0f*dt;
		MoveCamera(vMove);
	}

	if (dInput->GetKeyboardState(DIK_NUMPAD9))
	{
		XMVECTOR vMove = -GetForwardBackward();
		vMove *= 100.0f*dt;
		MoveCamera(vMove);
	}

	if (dInput->GetKeyboardState(DIK_NUMPAD7))
	{
		XMVECTOR vMove = GetForwardBackward();
		vMove *= 100.0f*dt;
		MoveCamera(vMove);
	}
	Camera::Update(dt);
}