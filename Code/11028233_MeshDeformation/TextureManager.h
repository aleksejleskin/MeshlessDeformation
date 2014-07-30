#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include "DxGraphics.h"

struct Texture
{
	string filename;

	ID3D11ShaderResourceView* texture;
	ID3D11SamplerState* samplerState;

	void Release()
	{
		texture->Release();
		samplerState->Release();
	};
};

class TextureManager
{
public:
	TextureManager();
	~TextureManager();

	bool LoadContent(DxGraphics* dx);
	void UnloadContent();

	Texture& GetTexture(string filename);
private:

	void LoadTextures(DxGraphics* dx, string filename);

	vector<Texture> m_textures;

	Texture m_errorTexture;
};

#endif