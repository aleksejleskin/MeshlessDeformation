#ifndef TEMPOBJECTTEST_H
#define TEMPOBJECTTEST_H

#include "ResourceManager.h"
#include "PhysicsObject.h"

class TempObjectTest : public PhysicsObject
{
public:
	TempObjectTest();
	~TempObjectTest();

	bool LoadContent(DxGraphics *dx, XMFLOAT3 position, ResourceManager& resource,
		float yaw, float pitch, float roll, float scale);
	void UnloadContent();
	
	void Update(float dt);
	void Render(DxGraphics *dx, Camera &cam);

private:
	bool BuildBuffers(DxGraphics *dx);
	bool BuildFX(DxGraphics *dx);

	Texture m_heightTex;
};

#endif