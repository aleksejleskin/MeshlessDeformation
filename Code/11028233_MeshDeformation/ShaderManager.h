#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

#include <iostream>
#include <string>

#include "DxGraphics.h"

using std::string;

struct Shader
{
	string filename;

	ID3D11VertexShader* vShader;
	ID3D11PixelShader* pShader;

	ID3DBlob* vBlob;
	ID3DBlob* pBlob;

	void Release()
	{
		ReleaseCOM(vShader);
		ReleaseCOM(pShader);
	}
};

class ShaderManager
{
public:
	ShaderManager();
	~ShaderManager();

	bool LoadContent(DxGraphics* dx);
	void UnloadContent();

	Shader& GetShader(string filename);

private:
	vector<Shader> m_shaders;

	Shader m_errorShader;

	void LoadShaders(DxGraphics* dx, string filename);
	bool CompileShader(ID3DBlob** shader, string filename, string entry, string shaderModel);
};

#endif