#ifndef CAMERA_H
#define CAMERA_H

#include <windows.h>
#include <xnamath.h>
#include <vector>
#include "MathHelper.h"

using std::vector;
using namespace MathHelper;

#include <iostream>
using namespace std;

#include <Eigen/Core>
using namespace Eigen;

class Camera
{
public:
	Camera();
	virtual ~Camera();

	virtual void Update(float dt);
	virtual void Initialise(XMFLOAT3 position, float yaw, float pitch, float roll,
		float fov, float aspectRatio, XMFLOAT3 target);

	void RebuildProjMatrix(float fov, float aspectRatio);
	//Getters
	XMMATRIX GetViewMatrix() const;
	XMMATRIX GetProjMatrix() const;
	XMVECTOR GetRotationQuaternion();//in Quaternion
	XMFLOAT3 GetPosition();
	//Setters

	void SetRotationQuaternion(XMVECTOR _quatRot);
	void SetFocalPointAndPosition(XMFLOAT3 &point, XMFLOAT3 &position);
	void SetAxis(XMVECTOR vLookAt, XMVECTOR vUp, XMVECTOR vRight);
	void MoveCamera(XMVECTOR &pos);
	void SetPosition(XMFLOAT3 pos);
	XMFLOAT4 EulerToQuaternion(float theta_z, float theta_y, float theta_x);
	//	void SetViewMat( XMMATRIX vMat );

	//current focus point
	XMVECTOR GetLookAt();

	//Get cameras local axis
	XMVECTOR GetRightLeft();
	XMVECTOR GetUpDown();
	XMVECTOR GetForwardBackward();

	void CalculateViewMatrix();

	//Setter
	void RotYaw(float angle);
	void RotPitch(float angle);
	void RotRoll(float angle);
	//change cameras focus point only
	void SetFocalPoint(XMFLOAT3 &v3FocalPoint);

protected:
	float m_yaw,
		m_pitch,
		m_roll,
		m_scale;

	XMVECTOR vPosition;
	XMMATRIX m_projMat,
		m_viewMat;

	XMFLOAT3 xUp, xRight;
	XMVECTOR m_QuatRot;
	XMVECTOR vFocalPoint;
};

#endif