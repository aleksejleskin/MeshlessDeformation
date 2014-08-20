/*
	Contains the game structure
*/
#ifndef GAME_H
#define GAME_H

#include <Windows.h>

#include "System.h"
#include "Timer.h"
#include "Input.h"
#include "ResourceManager.h"
#include "DebugCamera.h"
#include "Box.h"
#include "MeshDeformation.h"
#include <Windows.h>
//#include <Eigen/Eigen>
//#include <Eigen/Core>
//#include <Eigen/LU>
#include "Line.h"
#include "LightManager.h"


class Game
{
public:
	Game();
	~Game();

	bool Initialise(HINSTANCE hInstance, HINSTANCE pInstance,
		LPWSTR cmdLine, int cmdShow);
	void Run();
	void Shutdown();
	
	System GetSystem();
private:
	//calcualte frames per second
	INT64 CalculateFPS();
	INT64 m_fps;

	bool LoadContent();
	void UnloadContent();

	void Update(float dt);
	void DebugUpdate(float dt);

	void Render();
	void DebugRender();
	

	bool m_shutdown,
		m_debug,
		m_prevDebug;

	LightManager m_lightManager;
	System m_system;
	DebugCamera m_debugCam;
	Input* m_input;
	ResourceManager m_resources;
	Box* box;
	Box* m_obstacle2;
	MeshDeformation m_particleSystem;
	Line line;
	Box* m_wall;
	Box* m_terrain;
	Box* m_obstacle;
	bool camView;
};

#endif