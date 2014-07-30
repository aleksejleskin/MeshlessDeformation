#include "Terrain.h"

Terrain::Terrain()
{
}

Terrain::~Terrain()
{
}

bool Terrain::LoadTerrain(DxGraphics* dx, ResourceManager& resource)
{
	//Get the correct shader
	m_shader = resource.GetShaders().GetShader("colourShader.fx");
	if(m_shader.filename == "ERROR")
	{
		return false;
	}

	//Build the input layout for the terrain
	if(!BuildInputLayout(dx))
	{
		return false;
	}
	
	m_noise.CreateNoiseMap(8, 1.5f, 0.05f, 0.5f, 12345/*seeeeeed*/);

	TerrainLookupTables lookupTables;
	
	if(!lookupTables.LoadLookupTables())
	{
		return false;
	}
	
	//CHANGECHANGECHANGE
	terrainBlock = new TerrainBlock(m_shader, m_inputLayout, &lookupTables);

	terrainBlock->LoadBlock(dx, XMFLOAT3(-250.0f, -135.0f, 10.0f), m_noise.Get3DNoiseMap(), 0, 0, 0);

	return true;
}

void Terrain::UnloadContent()
{
	ReleaseCOM(m_inputLayout);
	terrainBlock->UnloadBlock();
}

void Terrain::Update(float dt)
{
	//if dist from end of biome create new biome real time//CHANGE
	terrainBlock->Update(dt);
}

void Terrain::Render(DxGraphics* dx, Camera &cam)
{
	terrainBlock->Render(dx, cam);
}

TerrainPerlinNoise Terrain::GetPerlinNoise()
{
	return m_noise;
}

bool Terrain::BuildInputLayout(DxGraphics* dx)
{
	//Describe how our vertexPos struct is to be interpreted
	D3D11_INPUT_ELEMENT_DESC inputDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D10_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	//Create the input layout
	HR(dx->GetDevice()->CreateInputLayout(inputDesc, 2, m_shader.vBlob->GetBufferPointer(),
		m_shader.vBlob->GetBufferSize(), &m_inputLayout));

	return true;
}