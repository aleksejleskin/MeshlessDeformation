/*
Contains the MathHelper namespace for ease of use functions
*/

#ifndef MATRIX_MATH_H
#define MATRIX_MATH_H

#include <Windows.h>
#include <xnamath.h>
#include <d3dx11.h>
#include "MathHelper.h"

#define a _11
#define b _12
#define c _13
#define d _21
#define e _22
#define f _23
#define g _31
#define h _32
#define i _33

namespace MatrixMath
{


	inline XMFLOAT3X3 MatrixToXMFLOAT3X3(XMMATRIX &M)
	{
		XMFLOAT3X3 f_M;

		for (int x = 0; x < 3; x++)
		{
			for (int y = 0; y < 3; y++)
			{
				f_M(x, y) = M(x, y);
			}
		}
		return f_M;

	}

	inline XMMATRIX XMFLOAT3X3ToMatrix(XMFLOAT3X3 &M)
	{
		XMMATRIX f_M;

		for (int x = 0; x < 3; x++)
		{
			for (int y = 0; y < 3; y++)
			{
				f_M(x, y) = M(x, y);
			}
		}
		return f_M;
	}


	inline XMFLOAT3X3 IdentityMatrix(XMFLOAT3X3 &M)
	{
		XMFLOAT3X3 iX = M;
		for (int row = 0; row < 3; row++)
		{
			for (int col = 0; col < 3; col++)
			{
				iX(row, col) = 0;
			}
		}
		iX._11 = 1;
		iX._22 = 1;
		iX._33 = 1;
		return iX;
	}

	inline Matrix3f volumeNormalize(Matrix3f M)
	{
		//calcauclte cubic root of matrix determinant
		double cbrt = pow(fabs((double)M.determinant()), 1. / 3);
		cbrt = ((double)M.determinant() < 0) ? -cbrt : cbrt;
		M = M / cbrt;

		return M;
	}


	inline float sign(float & X)
	{
		float iX = X;
		float out = 0;
		if (iX < 0) { out = -1; }
		if (iX == 0) { out = 0; }
		if (iX > 0) { out = 1; }

		return out;
	}

	inline XMFLOAT3 operator* (XMFLOAT3X3 & iA, XMFLOAT3 & irkPoint)
	{
		XMFLOAT3X3 A = iA;
		XMFLOAT3 rkPoint = irkPoint;
		XMFLOAT3 kProd;
		kProd.x =
			A(0, 0) * rkPoint.x +
			A(0, 1) * rkPoint.y +
			A(0, 2) * rkPoint.z;

		kProd.y =
			A(1, 0) * rkPoint.x +
			A(1, 1) * rkPoint.y +
			A(1, 2) * rkPoint.z;

		kProd.z =
			A(2, 0) * rkPoint.x +
			A(2, 1) * rkPoint.y +
			A(2, 2) * rkPoint.z;

		return kProd;
	}

	//multiply each index of matrix
	inline XMFLOAT3X3 operator^ (XMFLOAT3X3 & iA, XMFLOAT3X3 & iB)
	{
		XMFLOAT3X3 A = iA;
		XMFLOAT3X3 B = iB;
		XMFLOAT3X3 C;

		C._11 = A._11 * B._11;
		C._12 = A._12 * B._12;
		C._13 = A._13 * B._13;

		C._21 = A._21 * B._21;
		C._22 = A._22 * B._22;
		C._23 = A._23 * B._23;

		C._31 = A._31 * B._31;
		C._32 = A._32 * B._32;
		C._33 = A._33 * B._33;
		return C;
	}

	inline XMFLOAT3X3 operator*(XMFLOAT3X3 & A, XMFLOAT3X3 & B)
	{
		XMFLOAT3X3 iA, iB, iO;
		iA = A;
		iB = B;
		iO = IdentityMatrix(iO);
		for (int row = 0; row < 3; row++) {
			for (int col = 0; col < 3; col++) {
				// Multiply the row of A by the column of B to get the row, column of product.
				for (int inner = 0; inner < 3; inner++) {
					float num = iA(row, inner) * iB(inner, col);
					iO(row, col) += num;
				}
			}
		}
		return iO;
	}

	inline Vector3f rotateVect(Matrix3f & M, Vector3f& vect)
	{

		Vector3f tmp = vect;
		Vector3f out;
		for (size_t iRow = 0; iRow < 3; iRow++)
		{
			out.x() =
				M(0,0) * tmp.x() +
				M(0,1) * tmp.y() +
				M(0,2) * tmp.z();

			out.y() =
				M(1,0) * tmp.x() +
				M(1,1) * tmp.y() +
				M(1,2) * tmp.z();

			out.z() =
				M(2,0) * tmp.x() +
				M(2,1) * tmp.y() +
				M(2,2) * tmp.z();
		}

		return out;
	}

	inline XMFLOAT3 Normalize(XMFLOAT3 & iX)
	{
		XMFLOAT3 X = iX;
		float Length = 0;
		Length = sqrt((X.x*X.x) + (X.y*X.y) + (X.z*X.z));
		X.x = X.x / Length;
		X.y = X.y / Length;
		X.z = X.z / Length;
		return X;
	}

	inline XMFLOAT3 Cross(XMFLOAT3 & iA, XMFLOAT3 & iB)
	{
		XMFLOAT3 A = iA;
		XMFLOAT3 B = iB;
		XMFLOAT3 Out;

		Out.x = (-A.z*B.y + A.y*B.z);
		Out.y = (A.z*B.x - A.x*B.z);
		Out.z = (-A.y*B.x + A.x*B.y);
		return Out;

	}

	//multiply each index of matrix
	inline XMMATRIX operator^ (XMMATRIX & iA, XMMATRIX & iB)
	{
		XMMATRIX A = iA;
		XMMATRIX B = iB;
		XMMATRIX C;

		C._11 = A._11 * B._11;
		C._12 = A._12 * B._12;
		C._13 = A._13 * B._13;
		C._14 = A._14 * B._14;

		C._21 = A._21 * B._21;
		C._22 = A._22 * B._22;
		C._23 = A._23 * B._23;
		C._24 = A._24 * B._24;

		C._31 = A._31 * B._31;
		C._32 = A._32 * B._32;
		C._33 = A._33 * B._33;
		C._34 = A._34 * B._34;
		return C;
	}

	inline void Jacobi(XMFLOAT3X3&  mat, XMFLOAT3X3&  jmat, int j, int k) {
		// First, check if entries (j,k) is too small or not, if so, do nothing
		if (abs(mat(j,k)) > 1e-20) {
			// This is just some math to figure out cosine and sine necessary to zero out the two entries
			float tau = (mat(j,j) - mat(k,k)) / (2.0f*mat(j,k));
			float t = sign(tau) / (abs(tau) + sqrt(1 + tau*tau));
			float cc = 1 / sqrt(1 + t*t);
			float s = cc*t;
			// Build the rotation matrix
			XMFLOAT3X3 R = IdentityMatrix(R);
			R(j,j) = cc; R(k,k) = cc; R(j,k) = -s; R(k,j) = s;
			jmat = jmat* R; mat = mat^R;
			R(j,k) = s; R(k,j) = -s;
			mat = R^mat;
		}
	}

	inline XMMATRIX ComputeOptimumRotation(XMMATRIX& _A) 
	{
		XMMATRIX A = _A;
		XMMATRIX jmat = XMMatrixIdentity();
		XMMATRIX mat = XMMatrixMultiply(XMMatrixTranspose(A), A);
		
		XMFLOAT3X3 jMat,Mat;
		XMStoreFloat3x3(&jMat, jmat);
		XMStoreFloat3x3(&Mat, mat);
		// Do 5 iterations of Jacobi rotation
		for (int ii = 0; ii < 5; ii++) 
		{ 
			Jacobi(Mat, jMat, 0, 1);
			Jacobi(Mat, jMat, 0, 2);
			Jacobi(Mat, jMat, 1, 2);
		}
		jmat = XMLoadFloat3x3(&jMat);
		mat = XMLoadFloat3x3(&Mat);
		// A^tA == jmat^t mat jmat
		// OptimumR = A jmat^t sqrt(1/mat) jmat
		XMMATRIX optimumR = XMMatrixTranspose(XMMatrixMultiply(A, XMMatrixMultiply(XMMatrixTranspose(jmat), 
			XMMATRIX(
			XMVECTOR(jmat.r[0] / sqrt(mat(0,0))), 
			XMVECTOR(jmat.r[1] / sqrt(mat(1,1))), 
			XMVECTOR(jmat.r[2] / sqrt(mat(2, 2))), 
			XMVECTOR(jmat.r[2] / sqrt(mat(2, 2)))
			)
			)));
		const int first = 1, second = 2, third = 0;
		optimumR.r[first] = XMVector3Normalize(optimumR.r[first]);
		optimumR.r[third] = XMVector3Normalize(XMVector3Cross(optimumR.r[first], optimumR.r[second]));
		optimumR.r[second] = XMVector3Cross(optimumR.r[third], optimumR.r[first]);
		return XMMatrixTranspose(optimumR);
	}

	inline XMFLOAT3X3 operator- (XMFLOAT3X3 &A, XMFLOAT3X3 &B)
	{
		XMFLOAT3X3 Out;

		for (int x = 0; x < 3; x++)
		{
			for (int y = 0; y < 3; y++)
			{
				Out(x, y) = A(x, y) - B(x, y);
			}
		}
		return Out;
	}

	inline XMFLOAT3X3 operator*(int num, XMFLOAT3X3 &A)
	{
		XMFLOAT3X3 Out;

		for (int x = 0; x < 3; x++)
		{
			for (int y = 0; y < 3; y++)
			{
				Out(x, y) = A(x, y) * num;
			}
		}
		return Out;
	}
}

inline Vector3f XMFLOAT3toVector3f(XMFLOAT3 _value)
{
	Vector3f returnValue = Vector3f(_value.x, _value.y, _value.z);
	return returnValue;
}

inline XMFLOAT3 Vector3ftoXMFLOAT3(Vector3f _value)
{
	XMFLOAT3 returnValue = XMFLOAT3(_value.x(), _value.y(), _value.z());
	return returnValue;
}

inline XMMATRIX Matrix3ftoXMMATRIX(Matrix3f _eigenMat)
{
	XMMATRIX returnMatrix;
	for (int col = 0; col < 3; col++)
	{
		for (int row = 0; row < 3; row++)
		{
			returnMatrix(col, row) = _eigenMat(col, row);
		}
	}

	return returnMatrix;
}

inline Matrix3f XMMATRIXtoMatrix3f(XMMATRIX& _eigenMat)
{
	Matrix3f returnMatrix;
	for (int col = 0; col < 3; col++)
	{
		for (int row = 0; row < 3; row++)
		{
			returnMatrix(col, row) = _eigenMat(col, row);
		}
	}

	return returnMatrix;
}

#endif
