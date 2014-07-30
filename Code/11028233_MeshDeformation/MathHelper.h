/*
Contains the MathHelper namespace for ease of use functions
*/

#ifndef MATH_HELPER_H
#define MATH_HELPER_H

#include <Windows.h>
#include <xnamath.h>
#include <d3dx11.h>


namespace MathHelper
{

	inline float ValueDifferance(float value1, float value2)
	{
		float percentage = 0;
		percentage = (fabs((value1 - value2)) / ((value1 + value2) / 2)) * 100;
		return percentage;
	}

	inline XMVECTOR ZeroVector()
	{
		XMFLOAT3 zero = XMFLOAT3(0.0f, 0.0f, 0.0f);
		XMVECTOR vZero = XMLoadFloat3(&zero);

		return vZero;
	}

	//Helper function to radians to degrees
	inline float RadiansToDegrees(float radians)
	{
		return radians * (180 / XM_PI);
	}

	//Helper function to degrees to radians
	inline float DegreesToRadians(float degrees)
	{
		return degrees * (XM_PI / 180);
	}

	//Returns the direction vectors of a matrix
	inline XMFLOAT3 ForwardVec(XMFLOAT4X4 matrix)
	{
		return XMFLOAT3(matrix._31, matrix._32, matrix._33);
	}

	inline XMFLOAT3 BackwardVec(XMFLOAT4X4 matrix)
	{
		return XMFLOAT3(-matrix._31, -matrix._32, -matrix._33);
	}

	inline XMFLOAT3 UpVec(XMFLOAT4X4 matrix)
	{
		return XMFLOAT3(matrix._21, matrix._22, matrix._23);
	}

	inline XMFLOAT3 DownVec(XMFLOAT4X4 matrix)
	{
		return XMFLOAT3(-matrix._21, -matrix._22, -matrix._23);
	}

	inline XMFLOAT3 RightVec(XMFLOAT4X4 matrix)
	{
		return XMFLOAT3(matrix._11, matrix._12, matrix._13);
	}

	inline XMFLOAT3 LeftVec(XMFLOAT4X4 matrix)
	{
		return XMFLOAT3(-matrix._11, -matrix._12, -matrix._13);
	}

	//Overloading the XMFLOAT3 * operator
	inline XMFLOAT3 operator*(XMFLOAT3 l, XMFLOAT3 r)
	{
		float x = l.x * r.x;
		float y = l.y * r.y;
		float z = l.z * r.z;
		return XMFLOAT3(x, y, z);
	}

	//Overloading the XMFLOAT3 * float operator
	inline XMFLOAT3 operator*(XMFLOAT3 l, float r)
	{
		float x = l.x * r;
		float y = l.y * r;
		float z = l.z * r;

		return XMFLOAT3(x, y, z);
	}

	//Overloading the XMFLOAT3 + operator
	inline XMFLOAT3 operator+(XMFLOAT3 l, XMFLOAT3 r)
	{
		float x = l.x + r.x;
		float y = l.y + r.y;
		float z = l.z + r.z;
		return XMFLOAT3(x, y, z);
	}

	//Overloading the XMFLOAT3 - operator
	inline XMFLOAT3 operator-(XMFLOAT3 l, XMFLOAT3 r)
	{
		float x = l.x - r.x;
		float y = l.y - r.y;
		float z = l.z - r.z;
		return XMFLOAT3(x, y, z);
	}

	//Overloading the XMFLOAT3 / float operator
	inline XMFLOAT3 operator/(XMFLOAT3 l, float r)
	{
		float x = l.x / r;
		float y = l.y / r;
		float z = l.z / r;
		return XMFLOAT3(x, y, z);
	}

	//Overloading the XMFLOAT3 + float operatior
	inline XMFLOAT3 operator+(XMFLOAT3 l, float r)
	{
		float x = l.x + r;
		float y = l.y + r;
		float z = l.z + r;
		return XMFLOAT3(x, y, z);
	}

	//returns the negated vec
	inline XMFLOAT3 NegateVec(XMFLOAT3 vec)
	{
		return XMFLOAT3((vec.x * -1), (vec.y * -1), (vec.z * -1));
	}

	inline XMFLOAT3 NormaliseVec(XMFLOAT3 vec)
	{
		//Not sure if working
		float mag = sqrt(vec.x) + sqrt(vec.y) + sqrt(vec.z);
		vec.x /= mag;
		vec.y /= mag;
		vec.z /= mag;
		return vec;
	}

	//Returns a IdentityMatrix in the form of 4x4 due to XMMATRIX needing to be 16byte aligned
	inline XMFLOAT4X4 IdentityMat4x4(XMFLOAT4X4 mat)
	{
		XMMATRIX I = XMMatrixIdentity();
		XMStoreFloat4x4(&mat, I);
		return mat;
	}

};

#endif