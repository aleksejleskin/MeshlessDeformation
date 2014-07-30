#include "ShaderManager.h"

ShaderManager::ShaderManager()
{
	m_errorShader.filename = "ERROR";
}

ShaderManager::~ShaderManager()
{
}

bool ShaderManager::LoadContent(DxGraphics* dx)
{
	//Gets all the files in the Shaders DIR
	HANDLE hFind;
	WIN32_FIND_DATA findFileData;
	DWORD dError;

	hFind = FindFirstFile("Data/Shaders/*.*", &findFileData);
	
	if(hFind != INVALID_HANDLE_VALUE)
	{
		FindNextFile(hFind, &findFileData); // gets past the folder dir
		FindNextFile(hFind, &findFileData); // gets past the subfolder dir

		do
		{
			LoadShaders(dx, findFileData.cFileName);

		}while(FindNextFile(hFind, &findFileData));

		dError = GetLastError();
		//Close the HANDLE
		FindClose(hFind);
	}
	return true;
}

void ShaderManager::UnloadContent()
{
	for(unsigned int s = 0; s <(unsigned)m_shaders.size(); s++)
	{
		m_shaders[s].Release();
		m_shaders[s].vBlob->Release();
		m_shaders[s].pBlob->Release();
	}
}

void ShaderManager::LoadShaders(DxGraphics* dx, string filename)
{
	Shader newShader;

	// Set the filename to the same in the file
	newShader.filename = filename;

	filename = "Data/Shaders/" + filename;

	if(!CompileShader(&newShader.vBlob, filename, "VS_Main", "vs_5_0"))
	{
		MessageBoxA(NULL, "Failed to load vertex shader!", "ERROR", MB_OK);
	}
	if(!CompileShader(&newShader.pBlob, filename, "PS_Main", "ps_5_0"))
	{
		MessageBoxA(NULL, "Failed to load pixel shader!", "ERROR", MB_OK);
	}
	
	// Create the vertex shader
	HR(dx->GetDevice()->CreateVertexShader(newShader.vBlob->GetBufferPointer(),
		newShader.vBlob->GetBufferSize(), NULL, &newShader.vShader));

	// Create the pixel shader
	HR(dx->GetDevice()->CreatePixelShader(newShader.pBlob->GetBufferPointer(),
		newShader.pBlob->GetBufferSize(), NULL, &newShader.pShader));

	m_shaders.push_back(newShader);

}

bool ShaderManager::CompileShader(ID3DBlob** shader, string filename, string entry, string shaderModel)
{
	DWORD shaderFlags = 0;
	ID3DBlob* errorBuffer = 0;
	HRESULT hResult;

	//If debugging then compile shader in debug mode for error messages
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
#endif

	//Compile the shader using the determined filename, entry point, shader model
	hResult = D3DX11CompileFromFile(filename.c_str(), 0, 0, entry.c_str(), shaderModel.c_str(), 
		shaderFlags, 0, 0, shader, &errorBuffer, 0);

	//If the returned shader has errored then see what line in the .fx file
	if(errorBuffer != 0)
	{
		MessageBoxA(NULL, (char*)errorBuffer->GetBufferPointer(), 0, 0);
		errorBuffer->Release();
		errorBuffer = 0;
	}

	//If the compile failed completely then return a DXTRACE msg to link back to this function call
	if(FAILED ( hResult))
	{
		DXTRACE_MSG(__FILE__, (DWORD)__LINE__, hResult, "D3DX11CompileFromFile", true);
		return false;
	}

	return true;
}

Shader& ShaderManager::GetShader(string filename)
{
	for(unsigned int s = 0; s < (unsigned)m_shaders.size(); s++)
	{
		if(m_shaders[s].filename == filename)
		{
			return m_shaders[s];
		}
	}
	return m_errorShader;
}