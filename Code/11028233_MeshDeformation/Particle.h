#ifndef PARTICLE_H
#define PARTICLE_H

#include "GameObject.h"
#include "MatrixMath2.h"

using namespace MatrixMath;
class Particle : public GameObject
{
public:
	~Particle();
	Particle(string filename, 
		float mass, 
		float yaw,
		float pitch,
		float roll, 
		float scale);
	bool LoadContent(DxGraphics *dx, XMFLOAT3 position, ResourceManager& resource,
		float yaw, float pitch, float roll, float scale);

	void Update(float dt);
	void Render(DxGraphics* dx, Camera& cam);
	float setS;
	bool sellected;
	int m_cluster;
};


#endif