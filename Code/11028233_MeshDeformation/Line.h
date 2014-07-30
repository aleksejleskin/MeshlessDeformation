#ifndef LINE_H
#define LINE_H

#include "DxGraphics.h"
#include "Camera.h"
#include "ShaderManager.h"
#include "VertexTypes.h"
#include "ResourceManager.h"

class Line
{
public:
	Line();
	~Line();
	bool LoadContent(DxGraphics* _dx, ResourceManager & resource);
	void Update(float dt);
	void Render(DxGraphics* dx, Camera* cam);
	void UnloadContent();

	void SetSize(XMFLOAT3 _min, XMFLOAT3 _max);
	void SetColour(XMFLOAT3 _colour);
	void setPosition(XMFLOAT3 pos);

	float m_pitch, m_roll, m_yaw;
	XMFLOAT3 m_position;
	float m_scale;
private:

	ID3D11Buffer* vBuffer;
	ID3D11Buffer* iBuffer;
	ID3D11Buffer* worldCB;
	ID3D11Buffer* viewCB;
	ID3D11Buffer* projCB;

	ID3D11InputLayout* m_inputLayout;
	Shader		m_shader;

	vector<VertexPos> m_vertexList;
	vector<UINT>	m_indexList;

	XMFLOAT4X4 m_worldMat;

	DxGraphics* dx;
};
#endif