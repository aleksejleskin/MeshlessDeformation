#include "Box.h"

Box::Box() : GameObject(), m_currentObjectTime(0)
{
	m_collisionOBB.Extents.x =1;
	m_collisionOBB.Extents.y = 1;
	m_collisionOBB.Extents.z =1;
	rigid = false;
}

Box::~Box()
{
}

Box::Box(string filename, float mass, float yaw,
		 float pitch, float roll, float scale) 
{

	m_model->filename = filename;
	m_yaw = yaw;
	m_pitch = pitch;
	m_roll = roll;
	m_scale = scale;
}

bool Box::LoadContent(DxGraphics *dx, XMFLOAT3 position, ResourceManager& resource,
	float yaw, float pitch, float roll, float scale, bool _rigid)
{
	rigid = _rigid;
	//m_model->filename = _filename;
	if (!GameObject::LoadContent(dx, position, yaw, pitch, roll, scale))
	{
		return false;
	}
	if (!GameObject::BuildModel(dx->GetDevice(), m_model->filename, resource))
	{
		return false;
	}
	if (!GameObject::BuildShader("cellShader.fx", resource))
	{
		return false;
	}

	if (!m_cellShader.LoadShader(dx->GetDevice(), false))
	{
		return false;
	}

	if (!BuildInputLayout(dx))
	{
		return false;
	}
	if (rigid)
	{
	//	m_dynamicsWorld = resource.GetPhysicsManager()->GetWorld();
//		PhysicsObject::CreateRigidBody(100.0f, XMFLOAT3(0, -10, 0), btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
	}

	return true;
}

void Box::UnloadContent()
{
	GameObject::UnloadContent();
}

void Box::Update(float dt, DxGraphics *dx, Camera &camM, DirectInput* dInput)
{
	if (rigid)
	{
		//PhysicsObject::Update(dt);
	}
	else
	{
		GameObject::Update(dt);
	}
}

void Box::Render(DxGraphics* dx, Camera& cam, LightManager & lightManager)
{
	m_cellShader.Render(dx->GetImmediateContext(), m_inputLayout,
		m_vBuffer, m_iBuffer, m_model->indexCount, cam, lightManager, m_worldMat);

	//GameObject::Render(dx, cam);
}
