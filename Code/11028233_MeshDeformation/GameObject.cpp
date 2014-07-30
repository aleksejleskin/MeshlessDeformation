#include "GameObject.h"
#include "Camera.h"

GameObject::GameObject() : m_position(XMFLOAT3(0.0f, 0.0f, 0.0f)), m_scale(1.0f), m_yaw(0.0f),
m_pitch(0.0f), m_roll(0.0f), m_worldCB(0), m_projCB(0), m_viewCB(0), m_inputLayout(0)
{
	IdentityMat4x4(m_worldMat);
	m_model = new Model();
	m_shader = new Shader();
	initialPos = XMFLOAT3(0, 0, 0);


	//m_collisionOBB = new OrientedBox();
	m_collisionOBB.Extents.x = 1;
	m_collisionOBB.Extents.y = 1;
	m_collisionOBB.Extents.z = 1;
	m_collisionOBB.Orientation.w = 0.8f;
	m_collisionOBB.Orientation.w = 0.0f;
	m_collisionOBB.Orientation.w = -0.5f;
	m_collisionOBB.Orientation.w = 0.0f;
}

GameObject::~GameObject()
{
}

bool GameObject::LoadContent(DxGraphics *dx, XMFLOAT3 position,
	float yaw, float pitch, float roll, float scale)
{
	m_collisionOBB.Center = position;

	initialPos = position;
	m_position = position;
	m_yaw = yaw;
	m_pitch = pitch;
	m_roll = roll;
	m_scale = scale;

	//Describe the constant buffers used for the vertex shader
	D3D11_BUFFER_DESC cbd;
	ZeroMemory(&cbd, sizeof(D3D11_BUFFER_DESC));
	cbd.ByteWidth = sizeof(XMMATRIX);
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.Usage = D3D11_USAGE_DEFAULT;
	cbd.CPUAccessFlags = 0;
	cbd.MiscFlags = 0;

	//Create the constant buffers
	HR(dx->GetDevice()->CreateBuffer(&cbd, 0, &m_worldCB));
	HR(dx->GetDevice()->CreateBuffer(&cbd, 0, &m_projCB));
	HR(dx->GetDevice()->CreateBuffer(&cbd, 0, &m_viewCB));

	return true;
}

bool GameObject::LoadContent(DxGraphics* dx, XMFLOAT3 position,
	float yaw, float pitch, float roll, float scale, float mass,
	string modelFilename, string textureFilename, string shaderFilename,
	ResourceManager& resource)
{
	m_mass = mass;
	initialPos = position;
	m_position = position;
	m_yaw = yaw;
	m_pitch = pitch;
	m_roll = roll;
	m_scale = scale;

	//Create the constant buffers
	//Describe the constant buffers used for the vertex shader
	D3D11_BUFFER_DESC cbd;
	ZeroMemory(&cbd, sizeof(D3D11_BUFFER_DESC));
	cbd.ByteWidth = sizeof(XMMATRIX);
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.Usage = D3D11_USAGE_DEFAULT;
	cbd.CPUAccessFlags = 0;
	cbd.MiscFlags = 0;

	//Create the constant buffers
	HR(dx->GetDevice()->CreateBuffer(&cbd, 0, &m_worldCB));
	HR(dx->GetDevice()->CreateBuffer(&cbd, 0, &m_projCB));
	HR(dx->GetDevice()->CreateBuffer(&cbd, 0, &m_viewCB));

	if (!BuildModel(dx->GetDevice(), modelFilename, resource))
	{
		return false;
	}

	if (!BuildShader(shaderFilename, resource))
	{
		return false;
	}

	//Check to see if this model has a texture to build
	// greater then 3 because it needs .sbs at the end
	if (textureFilename.size() > 3)
	{
		if (!BuildTexture(textureFilename, resource))
		{
			return false;
		}
	}

	if (!BuildInputLayout(dx))
	{
		return false;
	}

	return true;
}

void GameObject::UnloadContent()
{
	ReleaseCOM(m_vBuffer);
	ReleaseCOM(m_iBuffer);
	ReleaseCOM(m_worldCB);
	ReleaseCOM(m_projCB);
	ReleaseCOM(m_viewCB);
	ReleaseCOM(m_inputLayout);
	//delete m_model;
	//delete m_shader;
}

void GameObject::Update(float dt)
{
	m_collisionOBB.Center = m_position;

	//Transform the worlmat by using rotation, translation and scale
	XMMATRIX rotationMat = XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, m_roll);
	XMMATRIX translationMat = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	XMMATRIX scaleMat = XMMatrixScaling(m_scale, m_scale, m_scale);

	XMMATRIX worldMat = XMLoadFloat4x4(&m_worldMat);
	worldMat = rotationMat * scaleMat * translationMat;
	XMStoreFloat4x4(&m_worldMat, worldMat);
}

void GameObject::Render(DxGraphics *dx, Camera &cam)
{
	//transpose the matrices for the shader
	//so that it changes from column to row
	XMMATRIX tWorldMat = XMMatrixTranspose(XMLoadFloat4x4(&m_worldMat));
	//cam.CalculateViewMatrix();
	XMMATRIX tViewMat = XMMatrixTranspose(cam.GetViewMatrix());
	XMMATRIX tProjMat = XMMatrixTranspose(cam.GetProjMatrix());

	//Set the buffers to the input assembler
	//	dx->GetImmediateContext()->IASetVertexBuffers(0, 1, &m_model->vBuffer, &m_model->stride, &m_model->offset);
	//	dx->GetImmediateContext()->IASetIndexBuffer(m_model->iBuffer, DXGI_FORMAT_R32_UINT, m_model->offset);
	dx->GetImmediateContext()->IASetVertexBuffers(0, 1, &m_vBuffer, &m_model->stride, &m_model->offset);
	dx->GetImmediateContext()->IASetIndexBuffer(m_iBuffer, DXGI_FORMAT_R32_UINT, m_model->offset);
	dx->GetImmediateContext()->IASetInputLayout(m_inputLayout);
	dx->GetImmediateContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//set the vertex shaders for this object draw
	dx->GetImmediateContext()->VSSetShader(m_shader->vShader, 0, 0);
	dx->GetImmediateContext()->PSSetShader(m_shader->pShader, 0, 0);

	if (m_texture.filename.size() != 0)
	{
		dx->GetImmediateContext()->PSSetShaderResources(0, 1, &m_texture.texture);
		dx->GetImmediateContext()->PSSetSamplers(0, 1, &m_texture.samplerState);
	}

	//Update the constant buffers to store the worldMat and view and projection Mat
	dx->GetImmediateContext()->UpdateSubresource(m_worldCB, 0, 0, &tWorldMat, 0, 0);
	dx->GetImmediateContext()->UpdateSubresource(m_projCB, 0, 0, &tViewMat, 0, 0);
	dx->GetImmediateContext()->UpdateSubresource(m_viewCB, 0, 0, &tProjMat, 0, 0);

	//Set the constant buffers for the vertex shaders to use for this run
	dx->GetImmediateContext()->VSSetConstantBuffers(0, 1, &m_worldCB);
	dx->GetImmediateContext()->VSSetConstantBuffers(1, 1, &m_projCB);
	dx->GetImmediateContext()->VSSetConstantBuffers(2, 1, &m_viewCB);

	dx->GetImmediateContext()->DrawIndexed(m_model->indexCount, 0, 0); // draws the indexed geometry
}

bool GameObject::BuildModel(ID3D11Device* pD3dDevice, string filename, ResourceManager& resource)
{
	//If the model does not exist then return false
	*m_model = resource.GetModels().GetModel(filename);
	if (m_model->filename == "ERROR")
	{
		return false;
	}

	//This is reworks to the need for seperate buffers
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(D3D11_BUFFER_DESC));
	D3D11_SUBRESOURCE_DATA vInitData;
	ZeroMemory(&vInitData, sizeof(D3D11_SUBRESOURCE_DATA));

	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.Usage = D3D11_USAGE_DYNAMIC;

	switch (m_model->vertexType)
	{
	case 1:
		m_model->stride = sizeof(VertexPos);
		m_model->offset = 0;

		vbd.ByteWidth = sizeof(VertexPos)* m_model->verticesPos.size();
		vInitData.pSysMem = &m_model->verticesPos[0];
		break;
	case 2:
		m_model->stride = sizeof(VertexPosNor);
		m_model->offset = 0;

		vbd.ByteWidth = sizeof(VertexPosNor)* m_model->verticesPosNor.size();
		vInitData.pSysMem = &m_model->verticesPosNor[0];
		break;
	case 3:
		m_model->stride = sizeof(VertexPosTex);
		m_model->offset = 0;

		vbd.ByteWidth = sizeof(VertexPosTex)* m_model->verticesPosTex.size();
		vInitData.pSysMem = &m_model->verticesPosTex[0];
		break;
	case 4:
		m_model->stride = sizeof(VertexPosNorTex);
		m_model->offset = 0;

		vbd.ByteWidth = sizeof(VertexPosNorTex)* m_model->verticesPosNorTex.size();
		vInitData.pSysMem = &m_model->verticesPosNorTex[0];
		break;
	};

	HRESULT hr = pD3dDevice->CreateBuffer(&vbd, &vInitData, &m_vBuffer);
	if (FAILED(hr))
	{
		return false;
	}

	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(D3D11_BUFFER_DESC));
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	ibd.Usage = D3D11_USAGE_DYNAMIC;
	ibd.ByteWidth = sizeof(UINT)* m_model->indicieList.size();

	D3D11_SUBRESOURCE_DATA iInitData;
	ZeroMemory(&iInitData, sizeof(iInitData));
	iInitData.pSysMem = &m_model->indicieList[0];

	hr = pD3dDevice->CreateBuffer(&ibd, &iInitData, &m_iBuffer);
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

bool GameObject::BuildShader(string filename, ResourceManager& resource)
{
	m_shader = &resource.GetShaders().GetShader(filename);

	if (m_shader->filename == "ERROR")
	{
		return false;
	}

	return true;
}

bool GameObject::BuildTexture(string filename, ResourceManager& resource)
{
	m_texture = resource.GetTextures().GetTexture(filename);

	if (m_texture.filename == "ERROR")
	{
		return false;
	}

	return true;
}

bool GameObject::BuildInputLayout(DxGraphics* dx)
{
	switch (m_model->vertexType)
	{
	case 1:
	{
			  //Describe how our vertexPos struct is to be interpreted
			  D3D11_INPUT_ELEMENT_DESC inputDesc[] =
			  {
				  { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				  { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D10_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
			  };

			  //Create the input layout
			  HR(dx->GetDevice()->CreateInputLayout(inputDesc, 2, m_shader->vBlob->GetBufferPointer(),
				  m_shader->vBlob->GetBufferSize(), &m_inputLayout));
	}
		break;
	case 2:
	{
			  //Describe how our vertexPos struct is to be interpreted
			  D3D11_INPUT_ELEMENT_DESC inputDesc[] =
			  {
				  { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				  { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D10_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				  { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D10_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
			  };

			  //Create the input layout
			  HR(dx->GetDevice()->CreateInputLayout(inputDesc, 3, m_shader->vBlob->GetBufferPointer(),
				  m_shader->vBlob->GetBufferSize(), &m_inputLayout));
	}
		break;

	case 3:
	{
			  //Describe how our vertexPos struct is to be interpreted
			  D3D11_INPUT_ELEMENT_DESC inputDesc[] =
			  {
				  { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				  { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D10_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
			  };

			  //Create the input layout
			  HR(dx->GetDevice()->CreateInputLayout(inputDesc, 2, m_shader->vBlob->GetBufferPointer(),
				  m_shader->vBlob->GetBufferSize(), &m_inputLayout));
	}
		break;

	case 4:
	{
			  //Describe how our vertexPos struct is to be interpreted
			  D3D11_INPUT_ELEMENT_DESC inputDesc[] =
			  {
				  { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				  { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D10_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				  { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D10_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
			  };

			  //Create the input layout
			  HR(dx->GetDevice()->CreateInputLayout(inputDesc, 3, m_shader->vBlob->GetBufferPointer(),
				  m_shader->vBlob->GetBufferSize(), &m_inputLayout));
	}
		break;
	}

	return true;
}

bool GameObject::ColourFade(DxGraphics* dx, float _speed, XMFLOAT4 _goalColour, float dt)
{
	//final colour of the model.
	XMFLOAT4 goalColour;
	//how fast to transition to that colour.
	float speed = _speed;

	//values used to detemine when the colour has been changed, for the whole model
	//highest vertecie index
	int vhighest = 0;
	//highest vertecie Y
	float vHighestPos = 0;

	//convert for dx to understand the colour.
	goalColour = _goalColour;
	goalColour.x = goalColour.x / 255;
	goalColour.y = goalColour.y / 255;
	goalColour.z = goalColour.z / 255;
	//alpha channel not used.
	goalColour.w = 1;

	unsigned int modelVerticeCount = m_model->verticesPosNor.size();

	for (unsigned int x = 0; x < modelVerticeCount; x++)
	{
		//RED
		if (m_model->verticesPosNor[x].colour.x > goalColour.x)
		{
			m_model->verticesPosNor[x].colour.x -= speed  *dt;
		}
		if (m_model->verticesPosNor[x].colour.x < goalColour.x)
		{
			m_model->verticesPosNor[x].colour.x += speed  *dt;
		}

		//GREEN
		if (m_model->verticesPosNor[x].colour.y > goalColour.y)
		{
			m_model->verticesPosNor[x].colour.y -= speed  *dt;
		}
		if (m_model->verticesPosNor[x].colour.y < goalColour.y)
		{
			m_model->verticesPosNor[x].colour.y += speed  *dt;
		}

		//BLUE
		if (m_model->verticesPosNor[x].colour.z > goalColour.z)
		{
			m_model->verticesPosNor[x].colour.z -= speed  *dt;
		}
		if (m_model->verticesPosNor[x].colour.z < goalColour.z)
		{
			m_model->verticesPosNor[x].colour.z += speed  *dt;
		}
	}
	RemapBuffers(dx);

	return false;
}

bool GameObject::ColourFadePos(DxGraphics* dx, float _speed, XMFLOAT4 _goalColour, float dt)
{
	//final colour of the model.
	XMFLOAT4 goalColour;
	//how fast to transition to that colour.
	float speed = _speed;

	//values used to detemine when the colour has been changed, for the whole model
	//highest vertecie index
	int vhighest = 0;
	//highest vertecie Y
	float vHighestPos = 0;

	//convert for dx to understand the colour.
	goalColour = _goalColour;
	goalColour.x = goalColour.x / 255;
	goalColour.y = goalColour.y / 255;
	goalColour.z = goalColour.z / 255;
	//alpha channel not used.
	goalColour.w = 1;

	unsigned int modelVerticeCount = m_model->verticesPos.size();

	for (unsigned int x = 0; x < modelVerticeCount; x++)
	{
		//RED
		if (m_model->verticesPos[x].colour.x > goalColour.x)
		{
			m_model->verticesPos[x].colour.x -= speed  *dt;
		}
		if (m_model->verticesPos[x].colour.x < goalColour.x)
		{
			m_model->verticesPos[x].colour.x += speed  *dt;
		}

		//GREEN
		if (m_model->verticesPos[x].colour.y > goalColour.y)
		{
			m_model->verticesPos[x].colour.y -= speed  *dt;
		}
		if (m_model->verticesPos[x].colour.y < goalColour.y)
		{
			m_model->verticesPos[x].colour.y += speed  *dt;
		}

		//BLUE
		if (m_model->verticesPos[x].colour.z > goalColour.z)
		{
			m_model->verticesPos[x].colour.z -= speed  *dt;
		}
		if (m_model->verticesPos[x].colour.z < goalColour.z)
		{
			m_model->verticesPos[x].colour.z += speed  *dt;
		}
	}
	//CHANGE COLOUR OF CAR
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	//	Disable GPU access to the vertex buffer data.
	auto m_d3dContext = dx->GetImmediateContext();

	m_d3dContext->Map(m_vBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	//	Update the vertex buffer here.
	memcpy(mappedResource.pData, &m_model->verticesPos[0], sizeof(VertexPos)* m_model->verticesPos.size());

	//	Reenable GPU access to the vertex buffer data.
	m_d3dContext->Unmap(m_vBuffer, 0);

	return false;
}

//GETTERS

XMFLOAT3 GameObject::GetPosition()
{
	return m_position;
}

float GameObject::GetYaw()
{
	return m_yaw;
}

float GameObject::GetPitch()
{
	return m_pitch;
}

float GameObject::GetRoll()
{
	return m_roll;
}

float GameObject::GetScale()
{
	return m_scale;
}
XMFLOAT4X4 GameObject::GetWorldMat() const
{
	return m_worldMat;
}

void GameObject::SetWorldMat(XMFLOAT4X4 _worldMat)
{
	m_worldMat = _worldMat;
}

Model& GameObject::GetModel()
{
	return *m_model;
}

ID3D11Buffer* GameObject::GetVertexBuffer()
{
	return m_vBuffer;
}

ID3D11Buffer* GameObject::GetIndexBuffer()
{
	return m_iBuffer;
}

ID3D11InputLayout* GameObject::GetInputLayout()
{
	return m_inputLayout;
}

int GameObject::GetIndexCount()
{
	return m_model->indexCount;
}

///SETTERS

void GameObject::SetPosition(XMFLOAT3 pos)
{
	m_position = pos;
}

void GameObject::SetPitch(float pitch)
{
	m_pitch = pitch;
}

void GameObject::SetYaw(float yaw)
{
	m_yaw = yaw;
}

void GameObject::SetRoll(float roll)
{
	m_roll = roll;
}

void GameObject::SetScale(float scale)
{
	m_scale = scale;
}

void GameObject::SetColour(XMFLOAT4 colour)
{
	for (unsigned int x = 0; x < m_model->verticesPosNor.size(); x++)
	{
		m_model->verticesPosNor[x].colour = colour;
	}
}

OrientedBox GameObject::GetCollisionOBB()
{
	return m_collisionOBB;
}

void GameObject::RemapBuffers(DxGraphics* dx)
{
	//CHANGE COLOUR OF CAR
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	//	Disable GPU access to the vertex buffer data.
	auto m_d3dContext = dx->GetImmediateContext();

	m_d3dContext->Map(m_vBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	//	Update the vertex buffer here.
	memcpy(mappedResource.pData, &m_model->verticesPosNor[0], sizeof(VertexPosNor)* m_model->verticesPosNor.size());

	//	Reenable GPU access to the vertex buffer data.
	m_d3dContext->Unmap(m_vBuffer, 0);
}
