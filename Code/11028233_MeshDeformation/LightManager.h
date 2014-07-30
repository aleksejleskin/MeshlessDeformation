//LightManager.h

#ifndef INCLUDE_GUARD_LIGHT_MANAGER_H
#define INCLUDE_GUARD_LIGHT_MANAGER_H

#include <Windows.h>
#include <xnamath.h>
#include <iostream>
#include <vector>
#include "Utilities.h"

using std::vector;

static const int MAX_NUMBER_OF_LIGHTS = 10;

//Light struct to define its direction, position and intensity
struct Light
{
	XMFLOAT3 lightPosition;
	//XMFLOAT3 lightFalloff;

	Light()
	{
		lightPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
	}
};

class LightManager
{
public:
	LightManager();
	~LightManager();

	void CreateLight(XMFLOAT3 position);
	Light& GetLight(int index);
	vector<Light> GetAllLights();

	//This is to fill the Constant Buffer
	vector<XMFLOAT4> GetLightsPositions();

	void ReleaseLights();
private:
	int m_numberOfLights;

	vector<Light> m_lights;
};

#endif