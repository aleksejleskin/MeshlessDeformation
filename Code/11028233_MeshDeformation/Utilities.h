/*
Contains a namespace for helper functions and macros for debugging 
*/

#ifndef UTILITIES_H
#define UTILITIES_H

#include <iostream>
#include <vector>
#include <Windows.h>
#include <xnamath.h>
#include <d3dx11.h>
#include <dxerr.h>
#include <string>
#include <sstream>

#include <iomanip>

using std::vector;
using std::string;
using std::ostringstream;
using std::setprecision;

//Macro to safely release COM objects
#define ReleaseCOM(x)			\
{								\
	if(x)						\
{								\
	x->Release();				\
	x = 0;						\
}								\
}								\

//Debugging HRESULT macro retuns where the HRESULT failed using DXTRACEW
#if defined(DEBUG) | defined(_DEBUG)
#ifndef HR
#define HR(x)											\
{														\
	HRESULT hr = (x);									\
	if(FAILED(hr))										\
{														\
	DXTraceW(__FILE__,(DWORD)__LINE__, hr, L#x, true);	\
}														\
}															
#endif
#else
//Define it again so out of debug mode it will do nothing
#ifndef HR
#define HR(x) (x)
#endif
#endif

namespace Utilities
{
	//Helper function to delete a vector filled with memory allocation
	template <typename T>
	inline void DeleteVector(vector<T* > &vectorA)
	{
		for(unsigned int i = 0; i < vectorA.size(); i++)
		{
			delete vectorA[i];
		}
		vectorA.clear();
	}

	//Converts XMFLOAT3 to string for debug output
	inline string XMFLOAT3ToString(XMFLOAT3 m)
	{
		float fX = m.x, 
			fY = m.y, 
			fZ = m.z;
		string sX, 
			sY, 
			sZ, 
			output;
		ostringstream ssX, 
			ssY, 
			ssZ;

		ssX << setprecision(2) << fX;
		sX = ssX.str();
		ssY << setprecision(2) << fY;
		sY = ssY.str();
		ssZ << setprecision(2) << fZ;
		sZ = ssZ.str();

		return output = "(" + sX + ", " + sY + ", " + sZ + ")";
	}
};

#endif