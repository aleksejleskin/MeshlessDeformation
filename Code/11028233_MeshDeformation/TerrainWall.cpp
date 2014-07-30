#include "TerrainWall.h"

TerrainWall::TerrainWall() : m_biome(0), m_width(50), m_height(20),
	m_position (XMFLOAT3(0.0f, 0.0f, 0.0f))
{
}

TerrainWall::~TerrainWall()
{
}

bool TerrainWall::LoadWall(int biome, XMFLOAT3 position)
{
	m_biome = biome;
	m_position = position;

	return true;
}

void TerrainWall::Update(float dt)
{

}

void TerrainWall::Render(DxGraphics* dx, Camera& cam)
{

}

void TerrainWall::UnloadWall()
{

}