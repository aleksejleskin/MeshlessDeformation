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
	m_lightManager.CreateLight(XMFLOAT3(10.0f, 15.0f, 10.0f));
	m_lightManager.CreateLight(XMFLOAT3(-10.0f, 15.0f, -10.0f));


	//eigen paralelize

	//Eigen::setNbThreads(4);


	//Eigen::initParallel();
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
		m_system.Shutdown();
		delete m_input;
		m_shutdown = true;
	}
}

bool Game::LoadContent()
{


	//Load all the game assets
	//box = new Box("BoxPreview.sbs", 1, 0, 0, 0, 0.05f);
	//box = new Box("Sport.sbs", 1, 0, 0, 0, 0.05f);
	//box = new Box("bigCluster.sbs", 1, 0, 0, 0, 0.05f);
	m_terrain = new Box("Ground.sbs", 1, 0, 0, 0, 0.05f);
	
	//box = new Box("Plane.sbs", 1, 0, 0, 0, 0.05f);
	//box = new Box("SuperPoly.sbs", 1, 0, 0, 0, 0.05f);
	box = new Box("Spider.sbs", 1, 0, 0, 0, 0.05f);
	//box = new Box("Soccer.sbs", 1, 0, 0, 0, 0.05f);



	//box = new Box("Truck.sbs", 1, 0, 0, 0, 0.05f);
	//box = new Box("lowBox.sbs", 1, 0, 0, 0, 0.05f);
	m_wall = new Box("Wall.sbs", 1, 1.0, 0, 0, 0.15f);
	m_obstacle = new Box("BoxPreview.sbs", 1, 0, 0, 0, 0.05f);
	m_obstacle2 = new Box("BoxPreview.sbs", 1, 0, 0, 0, 0.05f);

#define BIT(x) (1<<(x))
	enum collisiontypes {
		COL_BOX = BIT(0), //<Collide with nothing
		COL_SHPERE = BIT(1), //<Collide with ships
		COL_GROUND = BIT(2), //<Collide with walls
		COL_TEST = BIT(3)
	};

	int ShperesCollideWith = COL_BOX;
	int boxCollidesWith = COL_GROUND | COL_SHPERE | COL_BOX;
	int terrainCollidesWith = COL_SHPERE | COL_BOX | COL_TEST;
	int testBoxCollidesWith = COL_BOX ;

	m_resources.LoadContent(m_system.GetDX());
	line.LoadContent(m_system.GetDX(), m_resources);
	box->LoadContent(m_system.GetDX(), XMFLOAT3(0.0f, 0.0f, 0.0f), m_resources, 0, 0, 0, 1.0f);
	//box->ApplyPhysics(100.0f, XMFLOAT3(0, -10, 0), btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK, COL_TEST, testBoxCollidesWith);
	////box->GetRigidBody()->setCollisionFlags(btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
	//btCollisionShape* shape = new btBoxShape(btVector3(0.5, 0.5, 0.5));
	//box->GetRigidBody()->setCollisionShape(shape);




	m_obstacle->LoadContent(m_system.GetDX(), XMFLOAT3(0.0f, 10.0f, 20.0f), m_resources, 0, 0, 0, 3.0f);
	m_obstacle->ApplyPhysics(1000.0f, XMFLOAT3(0, -10, 0),  btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK, COL_BOX, boxCollidesWith);
	m_obstacle2->LoadContent(m_system.GetDX(), XMFLOAT3(0.0f, 10.0f, -20.0f), m_resources, 0, 0, 0, 3.0f);
	m_obstacle2->ApplyPhysics(1000.0f, XMFLOAT3(0, -10, 0), btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK, COL_BOX, boxCollidesWith);


	m_terrain->LoadContent(m_system.GetDX(), XMFLOAT3(0.0f, -6.0f, 0.0f), m_resources, 0, 0, 0, 2.0f);
	m_terrain->ApplyPhysics(0.0f, XMFLOAT3(0, 0, 0), btCollisionObject::CF_STATIC_OBJECT, COL_GROUND, terrainCollidesWith);

	m_debugCam.Initialise(XMFLOAT3(0.0f, 3.0f, -20.0f), 0.0f, 0.0f, 0.0f,
		XM_PIDIV4, m_system.GetAspectRatio(), XMFLOAT3(0.0f, 0.0f, 1.0f));

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

	if (m_input->GetDirectInput()->GetKeyboardState(DIK_W))
	{
		m_obstacle->GetRigidBody()->setLinearVelocity(btVector3(15, 0, 0));
		m_obstacle2->GetRigidBody()->setLinearVelocity(btVector3(-15, 0, 0));
	}
	if (m_input->GetDirectInput()->GetKeyboardState(DIK_S))
	{
		m_obstacle->GetRigidBody()->setLinearVelocity(btVector3(-15, 0, 0));
		m_obstacle2->GetRigidBody()->setLinearVelocity(btVector3(15, 0, 0));
	}
	if (m_input->GetDirectInput()->GetKeyboardState(DIK_A))
	{
		m_obstacle->GetRigidBody()->setLinearVelocity(btVector3(0, 0, +15));
		m_obstacle2->GetRigidBody()->setLinearVelocity(btVector3(0, 0, -15));
	}
	if (m_input->GetDirectInput()->GetKeyboardState(DIK_D))
	{
		m_obstacle->GetRigidBody()->setLinearVelocity(btVector3(0, 0, -15));
		m_obstacle2->GetRigidBody()->setLinearVelocity(btVector3(0, 0, 15));
	}

	if (m_input->GetDirectInput()->GetKeyboardState(DIK_F))
	{
		m_obstacle->GetRigidBody()->setLinearVelocity(btVector3(0, 0, 0));
		m_obstacle2->GetRigidBody()->setLinearVelocity(btVector3(0, 0, 0));
	}

	if (m_input->GetDirectInput()->GetKeyboardState(DIK_Q))
	{
		XMFLOAT3 pos = m_obstacle->GetPosition();
		pos.y += 0.2f;
		m_obstacle->SetPosition(pos);
		pos.y += 0.2f;
		pos = m_obstacle2->GetPosition();
		m_obstacle2->SetPosition(pos);
	}

	m_obstacle2->Update(dt, m_system.GetDX(), m_debugCam, m_input->GetDirectInput());
	m_obstacle->Update(dt, m_system.GetDX(), m_debugCam, m_input->GetDirectInput());
	m_terrain->Update(dt, m_system.GetDX(), m_debugCam, m_input->GetDirectInput());
	box->GameObject::Update(dt);
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
	m_obstacle2->Render(m_system.GetDX(), *cam, m_lightManager);
	m_obstacle->Render(m_system.GetDX(), *cam, m_lightManager);
	m_terrain->Render(m_system.GetDX(), *cam, m_lightManager);
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