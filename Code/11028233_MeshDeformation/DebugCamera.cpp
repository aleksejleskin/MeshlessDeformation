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
	//XMFLOAT3 pos = XMFLOAT3(0,0,0);
	//XMStoreFloat3( &pos, vPosition );

	//std::cout << pos.x << std::endl;
	CalculateViewMatrix();
	if (dInput->GetKeyboardState(DIK_UP))
	{
		XMVECTOR vMove = GetForwardBackward();
		vMove *= 100.0f * dt;
		MoveCamera(vMove);
	}

	if (dInput->GetKeyboardState(DIK_DOWN))
	{
		XMVECTOR vMove = -GetForwardBackward();
		vMove *= 100.0f*dt;
		MoveCamera(vMove);
	}

	if (dInput->GetKeyboardState(DIK_LEFT))
	{
		XMVECTOR vMove = -GetRightLeft();
		vMove *= 100.0f*dt;
		MoveCamera(vMove);
	}

	if (dInput->GetKeyboardState(DIK_RIGHT))
	{
		XMVECTOR vMove = GetRightLeft();
		vMove *= 100.0f*dt;
		MoveCamera(vMove);
	}

	m_aimX = m_mouseSens * (dInput->GetMouseX() / 1280.0f) * dt;
	m_aimY = -m_mouseSens * (dInput->GetMouseY() / 800.0f) * dt;

	RotPitch(m_aimY);
	RotYaw(m_aimX);

	Camera::Update(dt);
}