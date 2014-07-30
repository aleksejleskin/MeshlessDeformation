#ifndef TERRAIN_WALL_H
#define TERRAIN_WALL_H

#include "DxGraphics.h"
#include "Camera.h"

class TerrainWall
{
public:
	TerrainWall();
	~TerrainWall();

	bool LoadWall(int biome, XMFLOAT3 position);

	void Update(float dt);
	void Render(DxGraphics* dx, Camera& cam);

	void UnloadWall();

private:
	int m_biome,
		m_width,
		m_height;

	XMFLOAT3 m_position;
};

#endif