//LightManager.cpp

#include "LightManager.h"

LightManager::LightManager() : m_numberOfLights(0)
{
}

LightManager::~LightManager()
{
}


void LightManager::CreateLight(XMFLOAT3 position)
{
	if(m_numberOfLights < MAX_NUMBER_OF_LIGHTS)
	{
		Light pNewLight;
		pNewLight.lightPosition = position;

		m_lights.push_back(pNewLight);
		m_numberOfLights += 1;
	}
}

Light& LightManager::GetLight(int index)
{
	if((unsigned)index < m_lights.size() && index >= 0)
	{
		return m_lights[index];
	}
	else return m_lights[0];//shouldnt get called
}

vector<Light> LightManager::GetAllLights()
{
	return m_lights;
}

vector<XMFLOAT4> LightManager::GetLightsPositions()
{
	vector<XMFLOAT4> m_lightPositions;

	for(unsigned int i = 0; i < MAX_NUMBER_OF_LIGHTS; i++)
	{
		if(i < m_lights.size())
		{
			//push back an actual light position
			XMFLOAT4 pos;
			pos.x = m_lights[i].lightPosition.x;
			pos.y = m_lights[i].lightPosition.y;
			pos.z = m_lights[i].lightPosition.z;
			pos.w = 1.0f;

			m_lightPositions.push_back(pos);
		}
		else
		{
			//Push back an empty position
			m_lightPositions.push_back(XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
		}
	}

	//	push back the number of lights in a vector
	//	this is to make an ease of constant buffers
	m_lightPositions.push_back(XMFLOAT4((float)m_numberOfLights,
		(float)m_numberOfLights,
		(float)m_numberOfLights, 1.0f));

	return m_lightPositions;
}

void LightManager::ReleaseLights()
{
}