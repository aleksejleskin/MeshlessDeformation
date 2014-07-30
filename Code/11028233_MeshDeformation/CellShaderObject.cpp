//CellShaderObjects.cpp

#include "CellShaderObject.h"

CellShaderObject::CellShaderObject() : m_lightCB(0), m_invWorldCB(0), m_camPosCB(0),
	m_worldCB(0), m_viewCB(0), m_projCB(0), m_vShader(0), m_pShader(0), m_vBlob(0),
	m_vertexShaderFilename(""), m_pixelShaderFilename(""), m_isTextured(false)
{
}

CellShaderObject::~CellShaderObject()
{
}

bool CellShaderObject::LoadShader(ID3D11Device* device, bool isTextured)
{
	m_isTextured = isTextured;

	string filepath = "";
	if(isTextured)
	{
		filepath = "Data/Shaders/cellShaderTex.fx";
	}
	else
	{
		filepath = "Data/Shaders/cellShader.fx";
	}

	if(!CreateAndCompileVertexShader(device, filepath, "VS_Main", "vs_5_0"))
	{
		return false;
	}
	if(!CreateAndCompilePixelShader(device, filepath, "PS_Main", "ps_5_0"))
	{
		return false;
	}

	//Create the matrix constant buffers
	D3D11_BUFFER_DESC matrixDesc;
	ZeroMemory(&matrixDesc, sizeof(D3D11_BUFFER_DESC));
	matrixDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixDesc.ByteWidth = sizeof(XMMATRIX);
	matrixDesc.CPUAccessFlags = 0;
	matrixDesc.MiscFlags = 0;
	matrixDesc.StructureByteStride = 0;
	matrixDesc.Usage = D3D11_USAGE_DEFAULT;

	//Creates the constant buffers that has the size of a matrix
	HR(device->CreateBuffer(&matrixDesc, NULL, &m_invWorldCB));
	HR(device->CreateBuffer(&matrixDesc, NULL, &m_worldCB));
	HR(device->CreateBuffer(&matrixDesc, NULL, &m_viewCB));
	HR(device->CreateBuffer(&matrixDesc, NULL, &m_projCB));

	D3D11_BUFFER_DESC vectorDesc;
	ZeroMemory(&matrixDesc, sizeof(D3D11_BUFFER_DESC));
	vectorDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	// The + 1 is so that the last vector includes the number of lights in the scene;
	vectorDesc.ByteWidth = sizeof(XMFLOAT4) * (MAX_NUMBER_OF_LIGHTS + 1);
	vectorDesc.CPUAccessFlags = 0;
	vectorDesc.MiscFlags = 0;
	vectorDesc.StructureByteStride = 0;
	vectorDesc.Usage = D3D11_USAGE_DEFAULT;

	//Creates the constant buffers that has the size of a vector
	HR(device->CreateBuffer(&vectorDesc, NULL, &m_camPosCB));
	HR(device->CreateBuffer(&vectorDesc, NULL, &m_lightCB));

	return true;
}

bool CellShaderObject::LoadShader(ID3D11Device* device, ID3D11VertexShader* vShader,
	ID3D11PixelShader* pShader, bool isTextured)
{
	m_isTextured = isTextured;

	//Check to see if the vertex shader has mem
	if(vShader)
	{
		m_vShader = vShader;
	}
	else
	{
		return false;
	}

	//Check to see if the pixel shader has mem
	if(pShader)
	{
		m_pShader = pShader;
	}
	else
	{
		return false;
	}

	//Create the matrix constant buffers
	D3D11_BUFFER_DESC matrixDesc;
	ZeroMemory(&matrixDesc, sizeof(D3D11_BUFFER_DESC));
	matrixDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixDesc.ByteWidth = sizeof(XMMATRIX);
	matrixDesc.CPUAccessFlags = 0;
	matrixDesc.MiscFlags = 0;
	matrixDesc.StructureByteStride = 0;
	matrixDesc.Usage = D3D11_USAGE_DEFAULT;

	//Creates the constant buffers that has the size of a matrix
	HR(device->CreateBuffer(&matrixDesc, NULL, &m_invWorldCB));
	HR(device->CreateBuffer(&matrixDesc, NULL, &m_worldCB));
	HR(device->CreateBuffer(&matrixDesc, NULL, &m_viewCB));
	HR(device->CreateBuffer(&matrixDesc, NULL, &m_projCB));

	D3D11_BUFFER_DESC vectorDesc;
	ZeroMemory(&matrixDesc, sizeof(D3D11_BUFFER_DESC));
	vectorDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	// The + 1 is so that the last vector includes the number of lights in the scene;
	vectorDesc.ByteWidth = sizeof(XMFLOAT4) * (MAX_NUMBER_OF_LIGHTS + 1);
	vectorDesc.CPUAccessFlags = 0;
	vectorDesc.MiscFlags = 0;
	vectorDesc.StructureByteStride = 0;
	vectorDesc.Usage = D3D11_USAGE_DEFAULT;

	//Creates the constant buffers that has the size of a vector
	HR(device->CreateBuffer(&vectorDesc, NULL, &m_camPosCB));
	HR(device->CreateBuffer(&vectorDesc, NULL, &m_lightCB));

	return true;
}

bool CellShaderObject::CreateAndCompileVertexShader(ID3D11Device* d3dDevice, std::string filename,
		std::string entry, std::string shaderModel)
{
	DWORD shaderFlags = 0;
	ID3DBlob* errorBlob = NULL;

	//If we are in debug, then give us debug info on the compile of the shader
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
#endif

	//Compile the shader from a file
	HRESULT hr = D3DX11CompileFromFile(filename.c_str(),
		0, 0, entry.c_str(), shaderModel.c_str(), shaderFlags, 0, 0, &m_vBlob, &errorBlob, 0);
	
	//If the shader failed to compile then show error
	if(errorBlob != 0)
	{
		MessageBoxA(NULL, (char*)errorBlob->GetBufferPointer(), 0, 0);
		errorBlob->Release();
		errorBlob = 0;

		return false;
	}

	if(FAILED(hr))
	{
		return false;
	}

	//Create the vertex shader with the vblob so we can use for a input layout
	// if we need to
	hr = d3dDevice->CreateVertexShader(m_vBlob->GetBufferPointer(),
		m_vBlob->GetBufferSize(), NULL, &m_vShader);

	if(FAILED(hr))
	{
		return false;
	}

	return true;
}

bool CellShaderObject::CreateAndCompilePixelShader(ID3D11Device* d3dDevice, std::string filename,
		std::string entry, std::string shaderModel)
{
	DWORD shaderFlags = 0;
	ID3DBlob* psBlob = NULL;
	ID3DBlob* errorBlob = NULL;

	m_pixelShaderFilename = filename;

	//If we are in debug, then give us debug info on the compile of the shader
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
#endif

	//Compile the shader from a file
	HRESULT hr = D3DX11CompileFromFile(filename.c_str(),
		0, 0, entry.c_str(), shaderModel.c_str(), shaderFlags, 0, 0, &psBlob, &errorBlob, 0);
	
	//If the shader failed to compile then show error
	if(errorBlob != 0)
	{
		MessageBoxA(NULL, (char*)errorBlob->GetBufferPointer(), 0, 0);
		errorBlob->Release();
		errorBlob = 0;

		return false;
	}

	if(FAILED(hr))
	{
		return false;
	}

	//Create the pixel shader
	hr = d3dDevice->CreatePixelShader(psBlob->GetBufferPointer(),
		psBlob->GetBufferSize(), NULL, &m_pShader);

	//Release resource
	ReleaseCOM(psBlob);

	if(FAILED(hr))
	{
		return false;
	}

	return true;
}

void CellShaderObject::Render(ID3D11DeviceContext* d3dDeviceContext, ID3D11InputLayout* inputLayout,
		ID3D11Buffer* vertexBuffer, ID3D11Buffer* indexBuffer, UINT indexCount,
		Camera& camera, LightManager& lightManager, XMFLOAT4X4 worldMatrix)
{
	//Transpose the matrixs so the shader interprets them properly
	XMVECTOR det;
	XMMATRIX tWorldMat = XMMatrixTranspose(XMLoadFloat4x4(&worldMatrix));
	//camera.CalculateViewMatrix();
	XMMATRIX tViewMat = XMMatrixTranspose(camera.GetViewMatrix());
	XMMATRIX tProjMat = XMMatrixTranspose(camera.GetProjMatrix());
	//the inverse however does not need to be transposed
	XMMATRIX tInvWorld = XMMatrixInverse(&det, XMLoadFloat4x4(&worldMatrix));
	XMVECTOR camPos = XMLoadFloat3(&camera.GetPosition());

	UINT stride = sizeof(VertexPosNor);
	UINT offset = 0;

	d3dDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	d3dDeviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, offset);
	d3dDeviceContext->IASetInputLayout(inputLayout);
	d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	d3dDeviceContext->VSSetShader(m_vShader, 0, 0);
	d3dDeviceContext->PSSetShader(m_pShader, 0, 0);

	//update the constant buffers
	d3dDeviceContext->UpdateSubresource(m_camPosCB, 0, 0, &camPos, 0, 0);
	d3dDeviceContext->UpdateSubresource(m_worldCB, 0, 0, &tWorldMat, 0, 0);
	d3dDeviceContext->UpdateSubresource(m_invWorldCB, 0, 0, &tInvWorld, 0, 0);
	d3dDeviceContext->UpdateSubresource(m_viewCB, 0, 0, &tViewMat, 0, 0);
	d3dDeviceContext->UpdateSubresource(m_projCB, 0, 0, &tProjMat, 0, 0);
	d3dDeviceContext->UpdateSubresource(m_lightCB, 0, 0, &lightManager.GetLightsPositions()[0], 0, 0);

	//Set them to the vs shader
	d3dDeviceContext->VSSetConstantBuffers(0, 1, &m_worldCB);
	d3dDeviceContext->VSSetConstantBuffers(1, 1, &m_viewCB);
	d3dDeviceContext->VSSetConstantBuffers(2, 1, &m_projCB);

	d3dDeviceContext->VSSetConstantBuffers(3, 1, &m_lightCB);
	d3dDeviceContext->VSSetConstantBuffers(4, 1, &m_camPosCB);
	d3dDeviceContext->VSSetConstantBuffers(5, 1, &m_invWorldCB);

	//Set the light cb to the pixel shader
	d3dDeviceContext->PSSetConstantBuffers(3, 1, &m_lightCB);

	//Draw the model
	d3dDeviceContext->DrawIndexed(indexCount, 0, 0);
}

void CellShaderObject::Render(ID3D11DeviceContext* d3dDeviceContext, ID3D11InputLayout* inputLayout,
		ID3D11Buffer* vertexBuffer, ID3D11Buffer* indexBuffer, UINT indexCount, 
		ID3D11ShaderResourceView* textureSRV, ID3D11SamplerState* samplerState,
		Camera& camera, LightManager& lightManager, XMFLOAT4X4 worldMatrix)
{
	//Transpose the matrixs so the shader interprets them properly
	XMVECTOR det;
	XMMATRIX tWorldMat = XMMatrixTranspose(XMLoadFloat4x4(&worldMatrix));
	//camera.CalculateViewMatrix();
	XMMATRIX tViewMat = XMMatrixTranspose(camera.GetViewMatrix());
	XMMATRIX tProjMat = XMMatrixTranspose(camera.GetProjMatrix());
	//the inverse however does not need to be transposed
	XMMATRIX tInvWorld = XMMatrixInverse(&det, XMLoadFloat4x4(&worldMatrix));
	XMVECTOR camPos = XMLoadFloat3(&camera.GetPosition());

	UINT stride = sizeof(VertexPosNorTex);
	UINT offset = 0;

	d3dDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	d3dDeviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, offset);
	d3dDeviceContext->IASetInputLayout(inputLayout);
	d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	d3dDeviceContext->VSSetShader(m_vShader, 0, 0);
	d3dDeviceContext->PSSetShader(m_pShader, 0, 0);

	d3dDeviceContext->PSSetShaderResources(0, 1, &textureSRV);
	d3dDeviceContext->PSSetSamplers(0, 1, &samplerState);

	//update the constant buffers
	d3dDeviceContext->UpdateSubresource(m_camPosCB, 0, 0, &camPos, 0, 0);
	d3dDeviceContext->UpdateSubresource(m_worldCB, 0, 0, &tWorldMat, 0, 0);
	d3dDeviceContext->UpdateSubresource(m_invWorldCB, 0, 0, &tInvWorld, 0, 0);
	d3dDeviceContext->UpdateSubresource(m_viewCB, 0, 0, &tViewMat, 0, 0);
	d3dDeviceContext->UpdateSubresource(m_projCB, 0, 0, &tProjMat, 0, 0);
	d3dDeviceContext->UpdateSubresource(m_lightCB, 0, 0, &lightManager.GetLightsPositions()[0], 0, 0);

	//Set them to the vs shader
	d3dDeviceContext->VSSetConstantBuffers(0, 1, &m_worldCB);
	d3dDeviceContext->VSSetConstantBuffers(1, 1, &m_viewCB);
	d3dDeviceContext->VSSetConstantBuffers(2, 1, &m_projCB);

	d3dDeviceContext->VSSetConstantBuffers(3, 1, &m_lightCB);
	d3dDeviceContext->VSSetConstantBuffers(4, 1, &m_camPosCB);
	d3dDeviceContext->VSSetConstantBuffers(5, 1, &m_invWorldCB);

	//Set the light cb to the pixel shader
	d3dDeviceContext->PSSetConstantBuffers(3, 1, &m_lightCB);

	//Draw the model
	d3dDeviceContext->DrawIndexed(indexCount, 0, 0);
}

std::string CellShaderObject::GetVertexShaderFilename()
{
	return m_vertexShaderFilename;
}
std::string CellShaderObject::GetPixelShaderFilename()
{
	return m_pixelShaderFilename;
}

ID3DBlob* CellShaderObject::GetVertexBlob()
{
	return m_vBlob;
}

void CellShaderObject::UnloadShader()
{
	ReleaseCOM(m_vShader);
	ReleaseCOM(m_pShader);
	ReleaseCOM(m_vBlob);
	ReleaseCOM(m_lightCB);
	ReleaseCOM(m_invWorldCB);
	ReleaseCOM(m_camPosCB);
	ReleaseCOM(m_worldCB);
	ReleaseCOM(m_viewCB);
	ReleaseCOM(m_projCB);
}

//Old Draw with the model which has been changed however left in for keep sake incase we 
// find a better way

//void CellShaderObject::Render(ID3D11DeviceContext* d3dDeviceContext, ID3D11InputLayout* inputLayout,
//	Model* model, Camera& camera, LightManager& lightManager, XMFLOAT4X4 worldMatrix)
//{
//	//Transpose the matrixs so the shader interprets them properly
//	XMVECTOR det;
//	XMMATRIX tWorldMat = XMMatrixTranspose(XMLoadFloat4x4(&worldMatrix));
//	camera.CalculateViewMatrix();
//	XMMATRIX tViewMat = XMMatrixTranspose(camera.GetViewMatrix());
//	XMMATRIX tProjMat = XMMatrixTranspose(camera.GetProjMatrix());
//	//the inverse however does not need to be transposed
//	XMMATRIX tInvWorld = XMMatrixInverse(&det, XMLoadFloat4x4(&worldMatrix));
//	XMVECTOR camPos = XMLoadFloat3(&camera.GetPosition());
//
//	d3dDeviceContext->IASetVertexBuffers(0, 1, &model->vBuffer, &model->stride, &model->offset);
//	d3dDeviceContext->IASetIndexBuffer(model->iBuffer, DXGI_FORMAT_R32_UINT, model->offset);
//	d3dDeviceContext->IASetInputLayout(inputLayout);
//	d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//	d3dDeviceContext->VSSetShader(m_shader.vShader, 0, 0);
//	d3dDeviceContext->PSSetShader(m_shader.pShader, 0, 0);
//
//	//update the constant buffers
//	d3dDeviceContext->UpdateSubresource(m_camPosCB, 0, 0, &camPos, 0, 0);
//	d3dDeviceContext->UpdateSubresource(m_worldCB, 0, 0, &tWorldMat, 0, 0);
//	d3dDeviceContext->UpdateSubresource(m_invWorldCB, 0, 0, &tInvWorld, 0, 0);
//	d3dDeviceContext->UpdateSubresource(m_viewCB, 0, 0, &tViewMat, 0, 0);
//	d3dDeviceContext->UpdateSubresource(m_projCB, 0, 0, &tProjMat, 0, 0);
//	d3dDeviceContext->UpdateSubresource(m_lightCB, 0, 0, &lightManager.GetLightsPositions()[0], 0, 0);
//
//	//Set them to the vs shader
//	d3dDeviceContext->VSSetConstantBuffers(0, 1, &m_worldCB);
//	d3dDeviceContext->VSSetConstantBuffers(1, 1, &m_viewCB);
//	d3dDeviceContext->VSSetConstantBuffers(2, 1, &m_projCB);
//
//	d3dDeviceContext->VSSetConstantBuffers(3, 1, &m_lightCB);
//	d3dDeviceContext->VSSetConstantBuffers(4, 1, &m_camPosCB);
//	d3dDeviceContext->VSSetConstantBuffers(5, 1, &m_invWorldCB);
//
//	//Set the light cb to the pixel shader
//	d3dDeviceContext->PSSetConstantBuffers(3, 1, &m_lightCB);
//
//	//Draw the model
//	d3dDeviceContext->DrawIndexed(model->indexCount, 0, 0);
//}
