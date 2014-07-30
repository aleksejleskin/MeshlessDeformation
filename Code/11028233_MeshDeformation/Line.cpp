#include "Line.h"

Line::Line()
{
	m_pitch = 0, m_roll = 0, m_yaw = 0;
	m_position = XMFLOAT3(0, 0, 0);
	m_scale = 1;
}

Line::~Line()
{

}

void Line::UnloadContent()
{
	ReleaseCOM(vBuffer);
	ReleaseCOM(iBuffer);
	ReleaseCOM(worldCB);
	ReleaseCOM(viewCB);
	ReleaseCOM(projCB);
}

bool Line::LoadContent(DxGraphics* _dx, ResourceManager & resource)
{
	dx = _dx;

	XMFLOAT3 minPos = XMFLOAT3(3, 3, 3);
	XMFLOAT3 maxPos = XMFLOAT3(5, 5, 5);

	XMFLOAT3 high = XMFLOAT3(5,5,5),
		low = XMFLOAT3(0, 0, 0);
	VertexPos vPos;


	//BOT
	vPos.position = minPos;

	vPos.colour = XMFLOAT4(1, 0, 0, 0);
	m_vertexList.push_back(vPos);

	vPos.position = XMFLOAT3(minPos.x, minPos.y, maxPos.z);

	vPos.colour = XMFLOAT4(1, 0, 0, 0);
	m_vertexList.push_back(vPos);

	vPos.position = XMFLOAT3(maxPos.x, minPos.y, maxPos.z);

	vPos.colour = XMFLOAT4(1, 0, 0, 0);
	m_vertexList.push_back(vPos);

	vPos.position = XMFLOAT3(maxPos.x, minPos.y, minPos.z);

	vPos.colour = XMFLOAT4(1, 0, 0, 0);
	m_vertexList.push_back(vPos);


	//TOP
	vPos.position = XMFLOAT3(minPos.x, maxPos.y, minPos.z);
	
	vPos.colour = XMFLOAT4(1, 0, 0, 0);
	m_vertexList.push_back(vPos);

	vPos.position = XMFLOAT3(minPos.x, maxPos.y, maxPos.z);

	vPos.colour = XMFLOAT4(1, 0, 0, 0);
	m_vertexList.push_back(vPos);

	
	vPos.position = maxPos;
	vPos.colour = XMFLOAT4(1, 0, 0, 0);
	m_vertexList.push_back(vPos);

	vPos.position = XMFLOAT3(maxPos.x, maxPos.y, minPos.z);

	vPos.colour = XMFLOAT4(1, 0, 0, 0);
	m_vertexList.push_back(vPos);

	//BOTTOM
	m_indexList.push_back(0);
	m_indexList.push_back(1);

	m_indexList.push_back(1);
	m_indexList.push_back(2);

	m_indexList.push_back(2);
	m_indexList.push_back(3);

	m_indexList.push_back(3);
	m_indexList.push_back(0);

	//TOPPART
	m_indexList.push_back(4);
	m_indexList.push_back(5);

	m_indexList.push_back(5);
	m_indexList.push_back(6);

	m_indexList.push_back(6);
	m_indexList.push_back(7);

	m_indexList.push_back(7);
	m_indexList.push_back(4);

	//CONNECT
	//BOTTOM
	m_indexList.push_back(0);
	m_indexList.push_back(4);

	m_indexList.push_back(1);
	m_indexList.push_back(5);

	m_indexList.push_back(2);
	m_indexList.push_back(6);

	m_indexList.push_back(3);
	m_indexList.push_back(7);

	m_shader = resource.GetShaders().GetShader("colourShader.fx");

#pragma region vertexBuffer

	D3D11_BUFFER_DESC vbufferDesc;
	ZeroMemory(&vbufferDesc, sizeof(D3D11_BUFFER_DESC));

	vbufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbufferDesc.ByteWidth = sizeof(VertexPos)* m_vertexList.size();
	vbufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vbufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA vInit;
	ZeroMemory(&vInit, sizeof(D3D11_SUBRESOURCE_DATA));
	vInit.pSysMem = &m_vertexList[0];

	HRESULT HR = dx->GetDevice()->CreateBuffer(&vbufferDesc, &vInit, &vBuffer);
	if (FAILED(HR)) return false;

#pragma endregion

#pragma region IndexBuffer
	{
		D3D11_BUFFER_DESC ibufferDesc;
		ZeroMemory(&ibufferDesc, sizeof(D3D11_BUFFER_DESC));

		ibufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibufferDesc.ByteWidth = sizeof(UINT)* m_indexList.size();
		ibufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		ibufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		D3D11_SUBRESOURCE_DATA iInit;
		ZeroMemory(&iInit, sizeof(D3D11_SUBRESOURCE_DATA));
		iInit.pSysMem = &m_indexList[0];

		HRESULT HR = dx->GetDevice()->CreateBuffer(&ibufferDesc, &iInit, &iBuffer);
		if (FAILED(HR)) return false;
	}
#pragma endregion

#pragma region CBBUFFER
	{
		D3D11_BUFFER_DESC cbbufferDesc;
		ZeroMemory(&cbbufferDesc, sizeof(D3D11_BUFFER_DESC));

		cbbufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbbufferDesc.ByteWidth = sizeof(XMMATRIX);
		cbbufferDesc.Usage = D3D11_USAGE_DEFAULT;

		HRESULT HR = dx->GetDevice()->CreateBuffer(&cbbufferDesc, NULL, &worldCB);
		if (FAILED(HR)) return false;

		 HR = dx->GetDevice()->CreateBuffer(&cbbufferDesc, NULL, &viewCB);
		if (FAILED(HR)) return false;

		 HR = dx->GetDevice()->CreateBuffer(&cbbufferDesc, NULL, &projCB);
		if (FAILED(HR)) return false;
	}
#pragma endregion

	{
		D3D11_INPUT_ELEMENT_DESC inputDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D10_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		//Create the input layout
		HRESULT HR = dx->GetDevice()->CreateInputLayout(inputDesc, 2, m_shader.vBlob->GetBufferPointer(),
			m_shader.vBlob->GetBufferSize(), &m_inputLayout);
	}

	return true;
}

void Line::Update(float dt)
{
	//Transform the worlmat by using rotation, translation and scale
	XMMATRIX rotationMat = XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, m_roll);
	XMMATRIX translationMat = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	XMMATRIX scaleMat = XMMatrixScaling(m_scale, m_scale, m_scale);

	XMMATRIX worldMat = XMLoadFloat4x4(&m_worldMat);
	worldMat = rotationMat * scaleMat * translationMat;
	XMStoreFloat4x4(&m_worldMat, worldMat);	
}

void Line::Render(DxGraphics* dx, Camera* cam)
{
	XMMATRIX worldTrans,
		viewTrans,
		projTrans;

	XMMATRIX worldMat;
	worldMat = XMLoadFloat4x4(&m_worldMat);
	worldTrans = XMMatrixTranspose(worldMat);

	viewTrans = XMMatrixTranspose(cam->GetViewMatrix());

	projTrans = XMMatrixTranspose(cam->GetProjMatrix());


	UINT stride = sizeof(VertexPos);
	UINT offset = 0;
	dx->GetImmediateContext()->IASetVertexBuffers(0, 1, &vBuffer, &stride, &offset);
	dx->GetImmediateContext()->IASetIndexBuffer(iBuffer, DXGI_FORMAT_R32_UINT, offset);
	dx->GetImmediateContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);


	dx->GetImmediateContext()->IASetInputLayout(m_inputLayout);

	dx->GetImmediateContext()->UpdateSubresource(worldCB, 0, NULL, &worldTrans, 0, 0);
	dx->GetImmediateContext()->UpdateSubresource(viewCB, 0, NULL, &viewTrans, 0, 0);
	dx->GetImmediateContext()->UpdateSubresource(projCB, 0, NULL, &projTrans, 0, 0);


	dx->GetImmediateContext()->VSSetShader(m_shader.vShader, 0, 0);
	dx->GetImmediateContext()->PSSetShader(m_shader.pShader, 0, 0);

	dx->GetImmediateContext()->VSSetConstantBuffers(0, 1, &worldCB);
	dx->GetImmediateContext()->VSSetConstantBuffers(1, 1, &viewCB);
	dx->GetImmediateContext()->VSSetConstantBuffers(2, 1, &projCB);

	dx->GetImmediateContext()->DrawIndexed(m_indexList.size(), 0, 0);

	dx->GetImmediateContext()->VSSetShader(NULL, 0, 0);
	dx->GetImmediateContext()->PSSetShader(NULL, 0, 0);
}

void Line::SetSize(XMFLOAT3 _min, XMFLOAT3 _max)
{
	//BOTTOM
	m_vertexList[0].position = _min;
	m_vertexList[1].position = XMFLOAT3(_min.x, _min.y, _max.z);
	m_vertexList[2].position = XMFLOAT3(_max.x, _min.y, _max.z);
	m_vertexList[3].position = XMFLOAT3(_max.x, _min.y, _min.z);

	//TOP
	m_vertexList[4].position = XMFLOAT3(_min.x, _max.y, _min.z);
	m_vertexList[5].position = XMFLOAT3(_min.x, _max.y, _max.z);
	m_vertexList[6].position = _max;
	m_vertexList[7].position = XMFLOAT3(_max.x, _max.y, _min.z);

	//CHANGE COLOUR OF CAR
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	//	Disable GPU access to the vertex buffer data.
	auto m_d3dContext = dx->GetImmediateContext();

	m_d3dContext->Map(vBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	//	Update the vertex buffer here.
	memcpy(mappedResource.pData, &m_vertexList[0], sizeof(VertexPos)* m_vertexList.size());

	//	Reenable GPU access to the vertex buffer data.
	m_d3dContext->Unmap(vBuffer, 0);
}

void Line::SetColour(XMFLOAT3 _colour)
{
	for (unsigned int x = 0; x < m_vertexList.size(); x++)
	{
		m_vertexList[x].colour = XMFLOAT4(_colour.x, _colour.y, _colour.z, 0);
	}

	//CHANGE COLOUR OF CAR
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	//	Disable GPU access to the vertex buffer data.
	auto m_d3dContext = dx->GetImmediateContext();

	m_d3dContext->Map(vBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	//	Update the vertex buffer here.
	memcpy(mappedResource.pData, &m_vertexList[0], sizeof(VertexPos)* m_vertexList.size());

	//	Reenable GPU access to the vertex buffer data.
	m_d3dContext->Unmap(vBuffer, 0);
}

void Line::setPosition(XMFLOAT3 pos)
{
	m_position = pos;
}