//CellShaderObjects.h

// Call this load Shader and render if you want to render the model using a cell shader

#ifndef INCLUDE_GUARD_CELL_SHADER_OBJECT_H
#define INCLUDE_GUARD_CELL_SHADER_OBJECT_H

#include "ResourceManager.h"
#include "Camera.h"
#include "LightManager.h"

class CellShaderObject
{
public:
	CellShaderObject();
	~CellShaderObject();
	
	//Creates and comiles the shader
	bool LoadShader(ID3D11Device* device, bool isTextured);
	//For pre comiled shaders
	bool LoadShader(ID3D11Device* device, ID3D11VertexShader* vShader,
		ID3D11PixelShader* pShader, bool isTextured);

	//Pass in the current model
	//void Render(ID3D11DeviceContext* d3dDeviceContext, ID3D11InputLayout* inputLayout,
		//Model* model, Camera& camera, LightManager& lightManager, XMFLOAT4X4 worldMatrix);

	//Overloaded function to take in a vertex and index buffer, and its index amount
	void Render(ID3D11DeviceContext* d3dDeviceContext, ID3D11InputLayout* inputLayout,
		ID3D11Buffer* vertexBuffer, ID3D11Buffer* indexBuffer, UINT indexCount,
		Camera& camera, LightManager& lightManager, XMFLOAT4X4 worldMatrix);

	void Render(ID3D11DeviceContext* d3dDeviceContext, ID3D11InputLayout* inputLayout,
		ID3D11Buffer* vertexBuffer, ID3D11Buffer* indexBuffer, UINT indexCount, 
		ID3D11ShaderResourceView* textureSRV, ID3D11SamplerState* samplerState,
		Camera& camera, LightManager& lightManager, XMFLOAT4X4 worldMatrix);

	std::string GetVertexShaderFilename();
	std::string GetPixelShaderFilename();

	ID3DBlob* GetVertexBlob();

	void UnloadShader();
private:
	bool m_isTextured;

	//This could be done with a parent class of SHADER, which has all this functionallity
	// to save code duplication as both cell and compute shader have this function
	// along side the shader manager which could effectivly be removed
	bool CreateAndCompileVertexShader(ID3D11Device* d3dDevice, std::string filename,
		std::string entry, std::string shaderModel);
	bool CreateAndCompilePixelShader(ID3D11Device* d3dDevice, std::string filename,
		std::string entry, std::string shaderModel);

	std::string m_vertexShaderFilename;
	std::string m_pixelShaderFilename;

	ID3D11VertexShader* m_vShader;
	ID3D11PixelShader* m_pShader;

	//For the input layout incase we need it
	ID3DBlob* m_vBlob;

	ID3D11Buffer* m_lightCB;
	ID3D11Buffer* m_invWorldCB;
	ID3D11Buffer* m_camPosCB;

	ID3D11Buffer* m_worldCB;
	ID3D11Buffer* m_viewCB;
	ID3D11Buffer* m_projCB;
};

#endif