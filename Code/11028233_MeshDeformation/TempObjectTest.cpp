#include "TempObjectTest.h"

TempObjectTest::TempObjectTest() : PhysicsObject()
{
}

TempObjectTest::~TempObjectTest()
{

}

bool TempObjectTest::LoadContent(DxGraphics *dx, XMFLOAT3 position, ResourceManager& resources,
		float yaw, float pitch, float roll, float scale)
{
	//Load the constant buffers and assign all the attibutes needed
	if(!GameObject::LoadContent(dx, position, yaw, pitch, roll, scale))
	{
		return false;
	}

	//Build Model
	if(!GameObject::BuildModel("testTerrain.sbs", resources))
	{
		return false;
	}

	//Build Shader
	if(!GameObject::BuildShader("terrainShader.fx", resources))
	{
		return false;
	}

	if(!GameObject::BuildTexture("hMap.jpg", resources))
	{
		return false;
	}

	m_heightTex = resources.GetTextures().GetTexture("body.png");

	//Build the input layout
	if(!BuildInputLayout(dx))
	{
		return false;
	}
	
	//
	//maybe make a virtual physics object load content to do this?
	//

	m_mass = 10.0f;
	m_gravityMultiplier = 0.0f;
	m_dragForce = 50.0f;

	return true;
}

void TempObjectTest::UnloadContent()
{
	GameObject::UnloadContent();
}

void TempObjectTest::Update(float dt)
{
	PhysicsObject::Update(dt);
}

void TempObjectTest::Render(DxGraphics *dx, Camera &cam)
{

	//transpose the matrices for the shader
	//so that it changes from column to row
	XMMATRIX tWorldMat = XMMatrixTranspose(XMLoadFloat4x4(&m_worldMat));
	XMMATRIX tViewMat = XMMatrixTranspose(cam.GetViewMatrix());
	XMMATRIX tProjMat = XMMatrixTranspose(cam.GetProjMatrix());

	//Set the buffers to the input assembler
	dx->GetImmediateContext()->IASetVertexBuffers(0, 1, &m_model.vBuffer, &m_model.stride, &m_model.offset);
	dx->GetImmediateContext()->IASetIndexBuffer(m_model.iBuffer, DXGI_FORMAT_R32_UINT, m_model.offset);
	dx->GetImmediateContext()->IASetInputLayout(m_inputLayout);
	dx->GetImmediateContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//set the vertex shaders for this object draw
	dx->GetImmediateContext()->VSSetShader(m_shader.vShader, 0, 0);
	dx->GetImmediateContext()->PSSetShader(m_shader.pShader, 0, 0);

	dx->GetImmediateContext()->PSSetShaderResources(0, 1, &m_texture.texture);
	dx->GetImmediateContext()->PSSetSamplers(0, 1, &m_texture.samplerState);

	dx->GetImmediateContext()->PSSetShaderResources(1, 1, &m_heightTex.texture);
	dx->GetImmediateContext()->PSSetSamplers(1, 1, &m_heightTex.samplerState);
	
	//Update the constant buffers to store the worldMat and view and projection Mat
	dx->GetImmediateContext()->UpdateSubresource( m_worldCB, 0, 0, &tWorldMat, 0, 0);
	dx->GetImmediateContext()->UpdateSubresource( m_projCB, 0, 0, &tViewMat, 0, 0);	
	dx->GetImmediateContext()->UpdateSubresource( m_viewCB, 0, 0, &tProjMat, 0, 0);

	//Set the constant buffers for the vertex shaders to use for this run
	dx->GetImmediateContext()->VSSetConstantBuffers(0, 1, &m_worldCB);
	dx->GetImmediateContext()->VSSetConstantBuffers(1, 1, &m_projCB);
	dx->GetImmediateContext()->VSSetConstantBuffers(2, 1, &m_viewCB);

	dx->GetImmediateContext()->DrawIndexed(m_model.indexCount, 0, 0); // draws the indexed geometry

	//GameObject::Render(dx, cam);
}




