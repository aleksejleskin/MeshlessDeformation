#include "Game.h"

Game::Game() : m_system(1920, 1200), m_shutdown(false), m_debug(false),
m_prevDebug(false), camView(true)
{

}

Game::~Game()
{
	if (!m_shutdown)
	{
		Shutdown();
	}
}

bool Game::Initialise(HINSTANCE hInstance, HINSTANCE pInstance,
	LPWSTR cmdLine, int cmdShow)
{
	//Call system to initialise all the libarys needed
	if (!m_system.Initialise(hInstance, pInstance, cmdLine, cmdShow))
	{
		return false;
	}

	//Get the pointer of the directInput class thats been init
	m_input = new Input(m_system.GetDirectInput());
	m_lightManager.CreateLight(XMFLOAT3(0.0f, -5.0f, 5.0f));
	m_lightManager.CreateLight(XMFLOAT3(5.0f, 5.0f, 0.0f));


	//eigen paralelize

	Eigen::setNbThreads(4);


	Eigen::initParallel();
	return true;
}

void Game::Run()
{
	//if the game object fail to load then return before the loop is entered
	if (!LoadContent())
	{
		MessageBoxA(NULL, "Failed to load game objects", "ERROR!", MB_OK | MB_ICONERROR);
		return;
	}

	//Initalise the game timer
	m_system.GetGameTimer()->Reset();

	//The game loop is open until the system gets a WM_QUIT msg
	while ((!m_system.Done()) && (!m_input->GetQuit()))
	{
		//starts the game timer
		m_system.GetGameTimer()->Tick();

		//check input for this frame
		m_input->CheckInput();

		//if the F1 is pressed to enable debug mode
		m_debug = m_input->GetDebug();

		//Update the game
		Update(m_system.GetGameTimer()->GetDeltaTime());

		Render();
	}

	//Unload all the objects after the loop has closed
	UnloadContent();
}

void Game::Shutdown()
{
	if (!m_shutdown)
	{
		m_input->Shutdown();
		m_system.Shutdown();
		delete m_input;
		m_shutdown = true;
	}
}

bool Game::LoadContent()
{


	//Load all the game assets
	box = new Box("bigCluster.sbs", 1, 0, 0, 0, 0.05f);
	//box = new Box("BoxPreview.sbs", 1, 0, 0, 0, 0.05f);
	//box = new Box("Truck.sbs", 1, 0, 0, 0, 0.05f);
	//box = new Box("lowBox.sbs", 1, 0, 0, 0, 0.05f);
	m_wall = new Box("Wall.sbs", 1, 1.0, 0, 0, 0.15f);

	m_resources.LoadContent(m_system.GetDX());
	line.LoadContent(m_system.GetDX(), m_resources);
	box->LoadContent(m_system.GetDX(), XMFLOAT3(0.0f, 0.0f, 0.0f), m_resources, 0, 0, 0, 1.0f, false);


	m_debugCam.Initialise(XMFLOAT3(0.0f, 0.0f, -10.0f), 0.0f, 0.0f, 0.0f,
		XM_PIDIV4, m_system.GetAspectRatio(), XMFLOAT3(0.0f, 0.0f, 1.0f));

	m_vehicleCamera.Initialise(XMFLOAT3(0.0f, 2.0f, -17.0f), 0.0f, 0.0f, 0.0f,
		XM_PIDIV4, m_system.GetAspectRatio(), XMFLOAT3(0.0f, 0.0f, 5.0f));

	m_particleSystem.AddObject(box, m_system.GetDX(), m_resources, box->GetPosition());


	return true;
}

void Game::UnloadContent()
{
	m_resources.UnloadContent();
}

void Game::Update(float dt)
{
	m_resources.GetPhysicsManager()->Update(dt);
	//turn on wireframe
	if ((m_input->GetDirectInput()->GetKeyboardState(DIK_F2)) &&
		(!m_input->GetDirectInput()->GetKeyboardPrevState(DIK_F2)))
	{
		m_system.GetDX()->SetWireframeMode(!m_system.GetDX()->GetWireframeMode());
	}

	if ((m_input->GetDirectInput()->GetKeyboardState(DIK_F3)) &&
		(!m_input->GetDirectInput()->GetKeyboardPrevState(DIK_F3)))
	{
		if (camView == false) camView = true;
		else camView = false;
	}

	box->Update(dt, m_system.GetDX(), m_debugCam, m_input->GetDirectInput());
	m_particleSystem.Update(dt, m_system.GetDX(), m_debugCam, m_system.GetDirectInput());


	line.Update(dt);


	m_debugCam.Update(dt, m_input->GetDirectInput());
	m_debugCam.SetFocalPoint(box->GetPosition());


}

void Game::Render()
{

	//Clear the depthstencil and render target view
	m_system.GetDX()->StartRender();
	Camera* cam;
	if (camView)
	{
		cam = &m_debugCam;
	}
	else
	{
		cam = &m_vehicleCamera;
	}

	//line.Render(m_system.GetDX(), cam);
	box->Render(m_system.GetDX(), *cam, m_lightManager);
	//	m_wall->Render(m_system.GetDX(), *cam, m_lightManager);
	m_particleSystem.Render(m_system.GetDX(), *cam, m_lightManager);
	//m_scene.Render(m_system.GetDX(), *cam, m_lightManager);
	m_system.GetDX()->EndRender();

}


System Game::GetSystem()
{
	return m_system;
}

INT64 Game::CalculateFPS()
{
	static INT64 ilastTick, ilastFPS, iFPS;
	if (GetTickCount64() - ilastTick >= 1000)
	{
		ilastFPS = iFPS;
		iFPS = 0;
		ilastTick = GetTickCount64();
	}
	iFPS++;
	return ilastFPS;
}