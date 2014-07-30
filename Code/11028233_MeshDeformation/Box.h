#ifndef BOX_H
#define BOX_H

#include "GameObject.h"
#include <vector>
#include "MathHelper.h"
#include "DirectInput.h"
#include "Camera.h"
#include "CellShaderObject.h"

class Box : public GameObject
{
public:
	Box();
	~Box();

	Box(string filename,
		float mass, float yaw, float pitch, float row, float scale);

	bool LoadContent(DxGraphics *dx, XMFLOAT3 position, ResourceManager& resource,
		float yaw, float pitch, float roll, float scale, bool _rigid);
	void UnloadContent();

	void Update(float dt, DxGraphics *dx, Camera &cam, DirectInput* dInput);
	void Render(DxGraphics* dx, Camera& cam, LightManager & lightManager);

	XMVECTOR rotQuat;
private:
struct VertexPos
{
	XMFLOAT3 position;
	XMFLOAT4 colour;
};
	vector<XMFLOAT3> verticeslist;
	float m_currentObjectTime;
	UINT mPickedTriangle;
	
	float X, Y;
	CellShaderObject	m_cellShader;
	bool rigid;
};

#endif