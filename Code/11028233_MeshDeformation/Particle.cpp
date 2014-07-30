#include "Particle.h"
#include "MathHelper.h"

Particle::Particle(string filename, float mass, float yaw,
	float pitch, float roll, float scale) 
{
	m_model->filename = filename;
	m_yaw = yaw;
	m_pitch = pitch;
	m_roll = roll;
	m_scale = scale;

	//collision SetUp
	m_collisionOBB.Extents.x = 0.5f;
	m_collisionOBB.Extents.y = 0.5f;
	m_collisionOBB.Extents.z = 0.5f;

	sellected = false;
	m_cluster = 0;
}

Particle::~Particle()
{

}

bool Particle::LoadContent(DxGraphics *dx, XMFLOAT3 position, ResourceManager& resource,
	float yaw, float pitch, float roll, float scale)
{
	if (!GameObject::LoadContent(dx, position, yaw, pitch, roll, scale))
	{
		return false;
	}
	if (!GameObject::BuildModel(dx->GetDevice(), m_model->filename, resource))
	{
		return false;
	}
	if (!GameObject::BuildShader("colourShader.fx", resource))
	{
		return false;
	}

	if (!BuildInputLayout(dx))
	{
		return false;
	}
	return true;
}

void Particle::Update(float dt)
{
	m_scale = setS;
	GameObject::Update(dt);
}

void Particle::Render(DxGraphics* dx, Camera& cam)
{
	GameObject::Render(dx, cam);
}
