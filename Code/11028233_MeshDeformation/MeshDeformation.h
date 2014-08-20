#ifndef MESH_DEFORMATION_H
#define MESH_DEFORMATION_H

#include <iostream>
#include "GameObject.h"
#include "Particle.h"
#include <vector>
#include "DebugCamera.h"
#include "MathHelper.h"
#include "MatrixMath2.h"
#include "DirectInput.h"
#include <AntTweakBar.h>
#include <Eigen/Dense>
#include "Box.h"
#include "Line.h"
#include "LightManager.h"
#include "PhysicsManager.h"
//#include "Collisions.h"
using namespace MatrixMath;
using namespace Eigen;
using namespace std;


struct BufType
{
	float fa;
	Vector3f ff;
};

struct input
{
	Vector3f currentPosition;
};

struct output
{
	Vector3f relativePosition;

};

struct values
{
	Vector3f COM;
	float mass;
	float total;
};

struct Cluster
{
	Vector3f		m_deformedCenterOfMass;
	Vector3f		m_originalCenterOfMass;
	vector<int>		m_vertecies;
	XMFLOAT4X4		m_Aqq;
	XMFLOAT4X4		m_Apq;
};

struct ControlPoint
{
	//ogirianl position of a vertex
	Vector3f		 m_originalPos;
	//position of a particle relative to the center of mass of an obejct (deformed and original shape)
	Vector3f		 m_relativePosDeformed;
	Vector3f		 m_relativePosOriginal;
	//current position (deformed).
	Vector3f		 m_currentPos;
	//point where the current point tends to go towards.
	Vector3f		 m_goalPosition;
	//mass
	float			 m_mass;
	//relative position of a point in quadratic terms.
	MatrixXf		 Qmat;

	Vector3f		 velocity;
	Vector3f		 m_force;
};

struct MeshObject
{
	GameObject * object;

	//all vertecies;
	vector<ControlPoint*> m_controlPoints;
	//center of mass for the (original and deformed shape)
	Vector3f		m_originalCOM;
	Vector3f		m_deformedCOM;

	Matrix3f		m_Aqq;
	Matrix3f		m_Apq;

	// stiffness controll value
	float			m_alpha;
	// deformation ammount
	float			m_beta;
	MatrixXf		m_Aqq_tilde, m_Apq_tilde;

	float			m_originalVolume, m_deformedVolume;

	//Optimal Rotation Matrix
	Matrix3f Rotation;
	//Linear transformation Matrix
	Matrix3f A;
	//Quadratic transforamtion matrix
	MatrixXf AQM;
	//Clusters
	vector<Cluster> m_clusters;
	Vector3f m_clusterAmmount;
};


class MeshDeformation
{
public:
     MeshDeformation();
	~MeshDeformation();
	void Update(float dt, DxGraphics *dx, DebugCamera& cam, DirectInput* input);
	bool LoadContent(DxGraphics *dx, XMFLOAT3 position, ResourceManager& resource,
		float yaw, float pitch, float roll, float scale);


	void Render(DxGraphics* dx, Camera& cam, LightManager & lightManager);
	void AddObject(PhysicsObject* box, DxGraphics *dx, ResourceManager& resource, XMFLOAT3 pos);

private:
	//calcualte relative positions //Initial center of mass //Scaling matrix
	void InitialCalculations(MeshObject* obj);

	///COM
	//Calcualte original center of mass
	void CalcualteOriginalCom(MeshObject* object);
	//calculate displaced center of mass
	void CalculateDeformedCom(MeshObject* object);


	///RELATIVE POSITIONS OF PARTICLES
	//original
	void Calculate_q(MeshObject* object);
	//displaced
	void Calculate_p(MeshObject* object);
	//Quadratic Extension
	void Calculate_q_tilde(MeshObject* object);
	

	//MATRIX CALCUALTE
	//Calcualte Rotation Matrix
	void CalcualteApqMatrix(MeshObject* object);
	//Calcualte Scaling Matrix
	void CalcualteAqqMatrix(MeshObject* object);

	void CalcualteApqMatrix_Tilde(MeshObject& object);
	void CalcualteAqqMatrix_Tilde(MeshObject& object);

	//Differance in shape.
	void CalcualteOriginalMeshSum(MeshObject* object);
	void CalcualteDeformedMeshSum(MeshObject* object);


	//CLUSTERING
	//initial calcuations
	void CalcualteOriginalComCluster(Cluster& _cluster);
	void Calculate_qCluster(Cluster& _cluster);
	void CalcualteAqqMatrixCluster(Cluster& _cluster);
	//everyupdate calculations
	void CalculateDeformedComCluster(Cluster& _cluster);
	void Calculate_pCluster(Cluster& _cluster);
	void CalcualteApqMatrixCluster(Cluster& _cluster);
	void ClusterCalutate(DxGraphics* _dx, ResourceManager & resource);

	//update vertecies
	void PointUpdate(ControlPoint* point, float dt);
	//remap dx vertex buffers
	void BufferRemap(MeshObject* object, DxGraphics *dx);

	//calcualte volume of a mesh.
	float calcVolumeOriginal();
	float calcVolumeDeformed();


	//SHADER
	HRESULT CreateComputeDevice(ID3D11Device** ppDeviceOut, ID3D11DeviceContext** ppContextOut, BOOL bForceRef);
	HRESULT CreateComputeShader(LPCSTR pSrcFile, LPCSTR pFunctionName,
		ID3D11Device* pDevice, ID3D11ComputeShader** ppShaderOut);
	HRESULT CreateStructuredBuffer(ID3D11Device* pDevice, UINT uElementSize, UINT uCount, VOID* pInitData, ID3D11Buffer** ppBufOut);
	HRESULT CreateRawBuffer(ID3D11Device* pDevice, UINT uSize, VOID* pInitData, ID3D11Buffer** ppBufOut);
	HRESULT CreateBufferSRV(ID3D11Device* pDevice, ID3D11Buffer* pBuffer, ID3D11ShaderResourceView** ppSRVOut);
	HRESULT CreateBufferUAV(ID3D11Device* pDevice, ID3D11Buffer* pBuffer, ID3D11UnorderedAccessView** pUAVOut);
	ID3D11Buffer* CreateAndCopyToDebugBuf(ID3D11Device* pDevice, ID3D11DeviceContext* pd3dImmediateContext, ID3D11Buffer* pBuffer);

	void RunComputeShader(ID3D11DeviceContext* pd3dImmediateContext,
		ID3D11ComputeShader* pComputeShader,
		UINT nNumViews, ID3D11ShaderResourceView** pShaderResourceViews,
		ID3D11Buffer* pCBCS, void* pCSData, DWORD dwNumDataBytes,
		UINT nNumUnViews, ID3D11UnorderedAccessView** pUnorderedAccessView,
		UINT X, UINT Y, UINT Z);

	//SHADER 

	DxGraphics*					p_Dx;
	//--------------------------------------------------------------------------------------
	// Global variables
	//--------------------------------------------------------------------------------------
	ID3D11ComputeShader*        g_pCS;

	ID3D11Buffer*               g_pBuf0;
	ID3D11Buffer*               g_pBuf1;
	ID3D11Buffer*               g_pBufResult;

	ID3D11ShaderResourceView*   g_pBuf0SRV;
	ID3D11ShaderResourceView*   g_pBuf1SRV;
	ID3D11UnorderedAccessView*  g_pBufResultUAV;

	BufType g_vBuf0[5];
	BufType g_vBuf1[5];

	//new buffers
	input* g_vBufInput;
	values* g_vBufInput2;
	output* g_vBufOutput;


	//TweakBar
	TwBar*						myBar;

	//VARIABLES
	vector<Particle *>			m_particles ;
	//current deformation type of simualtion
	int							m_deformType;
	//current name of deformation
	string						m_deformationType;
	//Hold all the clusters, easy to update , render
	vector<Cluster>				m_clusterList;
	//checksum variables of mesh
	float						m_originalMeshSum, m_deformedMeshSum;
	//currently sellected control points
	vector<int>					m_sellected,m_static;
	//Lines that visualize the clusters.
	vector<Line*>				m_clusterLineList;
	//force that moves the control points
	float						 m_force;
	//percentage differance between original and deformed shape
	float						 m_differance;
	//Objects
	vector<MeshObject*>			 m_deformObjects;
	//mouse position
	float						 mouseX, mouseY;

	XMFLOAT3 rot;
	float Y;
	float Snorm;
	float fromBond;
	btDynamicsWorld*	m_dynamicsWorld;
	vector<btRigidBody*> m_collisionSpheres;
	PhysicsObject * p_box;
	bool stop;

	float Cyield;
	float Ccreep;
	//how much c in order to break;
	float Cmax;
};
#endif 