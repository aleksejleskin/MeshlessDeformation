#ifndef DEBUG_CAMERA_H
#define DEBUG_CAMERA_H

#include "Camera.h"
#include "DirectInput.h"
#include "MathHelper.h"

using namespace MathHelper;

class DebugCamera : public Camera
{
public:
	DebugCamera();
	~DebugCamera();
	void Update(float dt, DirectInput* dInput);
	void Initialise(XMFLOAT3 position, float yaw, float pitch, float roll,
		float fov, float aspectRatio, XMFLOAT3 target);

private:
	float m_mouseSens;
	float m_rotate;
	float m_aimX;
	float m_aimY;
};

#endif