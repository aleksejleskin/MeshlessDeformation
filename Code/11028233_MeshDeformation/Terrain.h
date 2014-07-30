#ifndef TERRAIN_H
#define TERRAIN_H

#include "DxGraphics.h"
#include "Camera.h"
#include "TerrainBlock.h"
#include "TerrainPerlinNoise.h"
#include "TerrainLookupTables.h"

class Terrain
{
public:
	Terrain();
	~Terrain();

	bool LoadTerrain(DxGraphics* dx, ResourceManager& resource);
	void UnloadContent();
	void Update(float dt);
	void Render(DxGraphics* dx, Camera& cam);
	TerrainPerlinNoise GetPerlinNoise();

private:
	//void PopulateTerrain();

	TerrainBlock* terrainBlock;
	vector<TerrainBlock> m_terrainBlocks;

	bool BuildInputLayout(DxGraphics* dx);

	TerrainPerlinNoise m_noise;
	Shader m_shader;
	ID3D11InputLayout* m_inputLayout;
};

#endif