#include "TextureManager.h"

TextureManager::TextureManager()
{
	m_errorTexture.filename = "ERROR";
}

TextureManager::~TextureManager()
{
}

bool TextureManager::LoadContent(DxGraphics *dx)
{
	//Go through the whole Data/Textures dir
	HANDLE hFind;
	WIN32_FIND_DATA findFileData;

	hFind = FindFirstFile("Data/Textures/*.*", &findFileData);
	
	if(hFind != INVALID_HANDLE_VALUE)
	{
		FindNextFile(hFind, &findFileData); // gets past the folder dir
		FindNextFile(hFind, &findFileData); // gets past the subfolder dir

		do
		{
			LoadTextures(dx, findFileData.cFileName);

		}while(FindNextFile(hFind, &findFileData));

		//Close the HANDLE
		FindClose(hFind);
	}

	return true;
}

void TextureManager::UnloadContent()
{
	for(unsigned int t = 0; t < (unsigned)m_textures.size(); t++)
	{
		m_textures[t].Release();
	}
}

void TextureManager::LoadTextures(DxGraphics* dx, string filename)
{
	Texture newTexture;

	newTexture.filename = filename;

	filename = "Data/Textures/" + filename; 

	HR(D3DX11CreateShaderResourceViewFromFile(dx->GetDevice(), filename.c_str(),
		NULL, NULL, &newTexture.texture, NULL));

	//Create the sampler desc, so the pixel shader now hows to interperate the texture data
	D3D11_SAMPLER_DESC sd;
	ZeroMemory( &sd, sizeof(D3D11_SAMPLER_DESC));
	sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; // if the U coord is over 1.0f then clamp which causes a texture wrap
	sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sd.MinLOD = 0;
	sd.MaxLOD = D3D11_FLOAT32_MAX;

	HR(dx->GetDevice()->CreateSamplerState(&sd, &newTexture.samplerState));

	m_textures.push_back(newTexture);
}

Texture& TextureManager::GetTexture(string filename)
{
	for(unsigned int t = 0; t < (unsigned)m_textures.size(); t++)
	{
		if(filename == m_textures[t].filename)
		{
			return m_textures[t];
		}
	}
	
	return m_errorTexture;
}