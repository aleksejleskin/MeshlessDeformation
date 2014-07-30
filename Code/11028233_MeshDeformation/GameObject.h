/*
Contains the base abstract GameObject class with all
the diffrent types of vertex structs
*/

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <iostream>
#include <string>
#include <fstream>
#include "DxGraphics.h"
#include "Camera.h"
#include "VertexTypes.h"
#include "MathHelper.h"
#include "xnacollision.h"
#include "ResourceManager.h"

using namespace XNA;
using std::string;
using std::ifstream;
using std::ios;
using std::getline;
using std::vector;

using namespace MathHelper;

class GameObject
{
public:
	GameObject();
	virtual ~GameObject();

	virtual bool LoadContent(DxGraphics *dx, XMFLOAT3 position,
		float yaw, float pitch, float roll, float scale);
	virtual bool LoadContent(DxGraphics* dx, XMFLOAT3 position,
		float yaw, float pitch, float roll, float scale, float mass,
		string modelFilename, string textureFilename,
		string shaderFilename, ResourceManager& resource);

	virtual void UnloadContent();

	virtual void Update(float dt);
	virtual void Render(DxGraphics *dx, Camera &cam);

	OrientedBox GetCollisionOBB();
	OrientedBox m_collisionOBB;

	XMFLOAT3 GetPosition();
	float GetYaw();
	float GetPitch();
	float GetRoll();
	float GetScale();
	XMFLOAT4X4 GetWorldMat() const;
	void SetWorldMat(XMFLOAT4X4 _worldMat);
	Model& GetModel();

	ID3D11Buffer* GetVertexBuffer();
	ID3D11Buffer* GetIndexBuffer();
	ID3D11InputLayout* GetInputLayout();
	int GetIndexCount();

	void SetPosition(XMFLOAT3 pos);
	void SetYaw(float yaw);
	void SetPitch(float pitch);
	void SetRoll(float roll);
	void SetScale(float scale);
	void SetColour(XMFLOAT4 colour);


	void RemapBuffers(DxGraphics* dx);
	bool ColourFade(DxGraphics* dx, float _speed, XMFLOAT4 _goalColour, float dt);
	bool ColourFadePos(DxGraphics* dx, float _speed, XMFLOAT4 _goalColour, float dt);
	XMFLOAT3 initialPos;
	float m_mass;
protected:
	Model* m_model;
	Shader* m_shader;
	Texture m_texture;

	XMFLOAT3 m_position;
	XMFLOAT4X4 m_worldMat;
	float m_scale,
		m_yaw,
		m_pitch,
		m_roll;

	ID3D11Buffer* m_vBuffer;
	ID3D11Buffer* m_iBuffer;

	ID3D11InputLayout* m_inputLayout;

	ID3D11Buffer* m_worldCB;
	ID3D11Buffer* m_projCB;
	ID3D11Buffer* m_viewCB;

	bool BuildInputLayout(DxGraphics* dx);
	bool BuildModel(ID3D11Device* pD3dDevice, string filename, ResourceManager& resource);
	//bool BuildModel(string filename, ResourceManager& resource);
	bool BuildShader(string filename, ResourceManager& resource);
	bool BuildTexture(string filename, ResourceManager& resource);
};

#endif