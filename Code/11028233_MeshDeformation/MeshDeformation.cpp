#include "MeshDeformation.h"

MeshDeformation::MeshDeformation()
{
	m_deformType = 0;
	m_deformationType = "basic";

	m_originalMeshSum = 0;
	m_deformedMeshSum = 0;

	m_force = 100.85f;
	m_differance = 0.0f;
	mouseX = 0;
	mouseY = 0;

	rot = XMFLOAT3(0, 0, 0);
	Y = 3;
}


MeshDeformation::~MeshDeformation()
{
	//remove the mesh data
	for each(MeshObject* obj in m_deformObjects)
	{
		for each(ControlPoint* ctp in obj->m_controlPoints)
		{
			delete ctp;
		}
		obj->m_controlPoints.clear();
		delete obj;
	}
	m_deformObjects.clear();

	//remove visual particles
	for each(Particle* point in m_particles)
	{
		delete point;
	}
	m_particles.clear();
}

void MeshDeformation::AddObject(GameObject* box, DxGraphics *dx, ResourceManager& resource, XMFLOAT3 pos)
{
	//add object to the list
	m_deformObjects.push_back(new MeshObject());
	MeshObject* GameObject = m_deformObjects[m_deformObjects.size() - 1];
	//initialize values;
	GameObject->object = box;
	GameObject->m_alpha = 0.9f;
	GameObject->m_beta = 0.5f;
	GameObject->m_deformedCOM = XMFLOAT3(0, 0, 0);
	GameObject->m_originalCOM = XMFLOAT3(0, 0, 0);
	GameObject->m_originalVolume = 0;
	GameObject->m_deformedVolume = 0;
	XMMATRIX ident = XMMatrixIdentity();
	XMStoreFloat4x4(&GameObject->m_Apq, ident);
	XMStoreFloat4x4(&GameObject->m_Aqq, ident);

	//Copy vertecies to local control points.
	unsigned int vertexCount =  GameObject->object->GetModel().verticesPosNor.size();

	for (unsigned int point = 0; point < vertexCount; point++)
	{
		GameObject->m_controlPoints.push_back(new ControlPoint());
		GameObject->m_controlPoints[point]->m_currentPos = GameObject->object->GetModel().verticesPosNor[point].position;
		GameObject->m_controlPoints[point]->m_goalPosition = GameObject->object->GetModel().verticesPosNor[point].position;
		GameObject->m_controlPoints[point]->m_originalPos = GameObject->object->GetModel().verticesPosNor[point].position;
		GameObject->m_controlPoints[point]->m_mass = 1.0f;
		GameObject->m_controlPoints[point]->m_relativePosDeformed = XMFLOAT3(0, 0, 0);
		GameObject->m_controlPoints[point]->m_relativePosOriginal = XMFLOAT3(0, 0, 0);
		GameObject->m_controlPoints[point]->velocity = XMFLOAT3(0, 0, 0);
		GameObject->m_controlPoints[point]->m_force = XMFLOAT3(0, 0, 0);
	}

	//objects to visualize Control points.
	for (unsigned int x = 0; x < GameObject->object->GetModel().verticesPosNor.size(); x++)
	{
		m_particles.push_back(new Particle("vSellect.sbs", 1, 1, 1, 1, 5));
		m_particles[x]->LoadContent(dx, pos, resource, 0, 0, 0, 1);
	}

	//sort the model into clusters and create lines to represent them.
	ClusterCalutate(dx, resource);

	//calculations that can be done with initial values. relativePosition, COM, MeshSum,SymmetricMatrix.
	InitialCalculations(GameObject);

#pragma region TweakBar

	//TweakBar
	TwInit(TW_DIRECT3D11, dx->GetDevice());
	TwWindowSize(900, 600);
	myBar = TwNewBar("Controls");


	TwAddVarRW(myBar, "Deformation Type", TW_TYPE_STDSTRING, &m_deformationType, "x");
	TwAddVarRW(myBar, "Deformation Ammount", TW_TYPE_FLOAT, &m_differance, "");
	TwAddVarRW(myBar, "Original Sum ", TW_TYPE_FLOAT, &m_originalMeshSum, "");
	TwAddVarRW(myBar, "Deformed Sum ", TW_TYPE_FLOAT, &m_deformedMeshSum, "");

	TwAddVarRW(myBar, "Volume original ", TW_TYPE_FLOAT, &m_deformObjects[m_deformObjects.size() - 1]->m_originalVolume, "");
	TwAddVarRW(myBar, "Volume deformed ", TW_TYPE_FLOAT, &m_deformObjects[m_deformObjects.size() - 1]->m_deformedVolume, "");

	TwAddVarRW(myBar, "Alpha (rest speed)", TW_TYPE_FLOAT, &m_deformObjects[m_deformObjects.size() - 1]->m_alpha, "min=0 max=1.0f step=0.1");
	TwAddVarRW(myBar, "Beta (deform ammount)", TW_TYPE_FLOAT, &m_deformObjects[m_deformObjects.size() - 1]->m_beta, "min=0 max=1.0f step=0.1");
	TwAddVarRW(myBar, "Force", TW_TYPE_FLOAT, &m_force, "min=0 max=10 step=0.5");
	//	TwAddVarRW(myBar, "FPS", TW_TYPE_INT32, &m_fps, "");

#pragma endregion

	p_Dx = dx;
	CreateComputeShader("Deform.hlsl", "CSMain", p_Dx->GetDevice(), &g_pCS);

	// INITIALZIE VERT
	int NUM_ELEMENTS = m_deformObjects[m_deformObjects.size() - 1]->m_controlPoints.size();
	g_vBufInput = new input[NUM_ELEMENTS];
	for (int z = 0; z < NUM_ELEMENTS; z++)
	{
		g_vBufInput[z].currentPosition = m_deformObjects[m_deformObjects.size() - 1]->m_controlPoints[z]->m_originalPos;
	}

	//initial values
	g_vBufInput2 = new values;
	g_vBufInput2->COM = XMFLOAT3(0, 0, 0);
	g_vBufInput2->mass = 0;
	g_vBufInput2->total = NUM_ELEMENTS;

	g_vBufOutput = new output[NUM_ELEMENTS];



	//create structured buffers
	CreateStructuredBuffer(p_Dx->GetDevice(), sizeof(input), NUM_ELEMENTS, &g_vBufInput[0], &g_pBuf0);
	CreateStructuredBuffer(p_Dx->GetDevice(), sizeof(values), 1, &g_vBufInput2, &g_pBuf1);
	CreateStructuredBuffer(p_Dx->GetDevice(), sizeof(output), NUM_ELEMENTS, NULL, &g_pBufResult);

	//create buffer views
	CreateBufferSRV(p_Dx->GetDevice(), g_pBuf0, &g_pBuf0SRV);
	CreateBufferUAV(p_Dx->GetDevice(), g_pBuf1, &g_pBuf1SRV);
	CreateBufferUAV(p_Dx->GetDevice(), g_pBufResult, &g_pBufResultUAV);

	//run shader
	const int buffInputCount = 1;
	const int buffOutputCount = 2;
	ID3D11ShaderResourceView* aRViews[buffInputCount] = { g_pBuf0SRV };
	ID3D11UnorderedAccessView* uRViews[buffOutputCount] = { g_pBufResultUAV  , g_pBuf1SRV};
	RunComputeShader(p_Dx->GetImmediateContext(), g_pCS, buffInputCount, aRViews, NULL, NULL, 0, buffOutputCount, uRViews, 1, 1, 1);






	//Copy from gpu to local 
	ID3D11Buffer* debugbuf = CreateAndCopyToDebugBuf(p_Dx->GetDevice(), p_Dx->GetImmediateContext(), g_pBufResult);
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	output *p;
	p_Dx->GetImmediateContext()->Map(debugbuf, 0, D3D11_MAP_READ, 0, &MappedResource);

	// Set a break point here and put down the expression "p, 1024" in your watch window to see what has been written out by our CS
	// This is also a common trick to debug CS programs.
	p = (output*)MappedResource.pData;


	
		//Copy from gpu to local 
		debugbuf = CreateAndCopyToDebugBuf(p_Dx->GetDevice(), p_Dx->GetImmediateContext(), g_pBuf1);
		values *pa;
		p_Dx->GetImmediateContext()->Map(debugbuf, 0, D3D11_MAP_READ, 0, &MappedResource);

		// Set a break point here and put down the expression "p, 1024" in your watch window to see what has been written out by our CS
		// This is also a common trick to debug CS programs.
		pa = (values*)MappedResource.pData;
	


}

void MeshDeformation::Update(float dt, DxGraphics *dx, DebugCamera& cam, DirectInput* input)
{
	//differance in percentage between original and deformed shape.
	m_differance = ValueDifferance(m_originalMeshSum, m_deformedMeshSum);
	m_deformObjects[m_deformObjects.size() - 1]->m_deformedVolume = calcVolumeDeformed();

	rot.x += 10 * dt;
	//Transform the worlmat by using rotation, translation and scale
	XMMATRIX rotationMat = XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z);
	XMFLOAT3 m_position = m_particles[48]->GetPosition();
	XMMATRIX translationMat = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	XMMATRIX scaleMat = XMMatrixScaling(1, 1, 1);
	XMMATRIX translationMatZero = XMMatrixTranslation(0,0,0);

	XMMATRIX worldMat = XMMatrixIdentity();
	worldMat = translationMat * rotationMat  * scaleMat * translationMat;
	XMFLOAT4X4 mat;
	XMStoreFloat4x4(&mat, worldMat);
	m_particles[48]->SetWorldMat(mat);

#pragma region RAYTRACE
	{
		if (mouseX == 0 && mouseY == 0)
		{
			mouseX = input->GetMouseX();
			mouseY = input->GetMouseY();
		}


		mouseX += input->GetMouseX();
		mouseY += input->GetMouseY();

		XMVECTOR pickRayInViewSpaceDir = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		XMVECTOR pickRayInViewSpacePos = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

		float PRVecX, PRVecY, PRVecZ;

		//Transform 2D pick position on screen space to 3D ray in View space
		PRVecX = (((2.0f * mouseX) / 1920) - 1) / cam.GetProjMatrix()(0, 0);
		PRVecY = -(((2.0f * mouseY) / 1200) - 1) / cam.GetProjMatrix()(1, 1);
		PRVecZ = 1.0f;	//View space's Z direction ranges from 0 to 1, so we set 1 since the ray goes "into" the screen

		pickRayInViewSpaceDir = XMVectorSet(PRVecX, PRVecY, PRVecZ, 0.0f);

		// Transform 3D Ray from View space to 3D ray in World space
		XMMATRIX pickRayToWorldSpaceMatrix;
		XMVECTOR matInvDeter;	//We don't use this, but the xna matrix inverse function requires the first parameter to not be null

		pickRayToWorldSpaceMatrix = XMMatrixInverse(&matInvDeter, cam.GetViewMatrix());	//Inverse of View Space matrix is World space matrix

		XMVECTOR pickRayInWorldSpacePos = XMVector3TransformCoord(pickRayInViewSpacePos, pickRayToWorldSpaceMatrix);
		XMVECTOR pickRayInWorldSpaceDir = XMVector3TransformNormal(pickRayInViewSpaceDir, pickRayToWorldSpaceMatrix);


		if (input->GetKeyboardState(DIK_T))
		{
			mouseX = 0;
			mouseY = 0;
		}

		if (input->GetKeyboardState(DIK_X))
		{
		//	m_deformObjects[m_deformObjects.size() - 1]->m_controlPoints[48]->m_force = XMFLOAT3(100, 100, 100);
			//Y += 0.5f*dt;
		}
		if (input->GetKeyboardState(DIK_C))
		{
			//m_deformObjects[m_deformObjects.size() - 1]->m_controlPoints[48]->m_force = XMFLOAT3(0, 10, 0);
			Y -= 0.5f*dt;
		}

		float dist = 0;




		if (input->GetKeyboardState(DIK_NUMPAD1))
		{
			m_sellected.clear();
		}

		if (input->GetKeyboardState(DIK_NUMPAD2))
		{
			for (int x = 0; x < m_sellected.size(); x++)
			{
				m_static.push_back(m_sellected[x]);
			}
			m_sellected.clear();
		}

		

		for each (MeshObject* obj in m_deformObjects)
		{
			for (unsigned int point = 0; point < obj->m_controlPoints.size(); point++)
			{
				XMFLOAT3 poss = obj->m_controlPoints[point]->m_currentPos;

				if (IntersectRayOrientedBox(pickRayInWorldSpacePos, pickRayInWorldSpaceDir, &m_particles[point]->GetCollisionOBB(), &dist))
				{
					if (input->GetKeyboardState(DIK_NUMPAD0))
					{
						m_sellected.push_back(point);
					}

					m_particles[point]->setS = 3; //set scale
				}
				else
				{
					m_particles[point]->setS = 1;
				}

				for (unsigned int t = 0; t < m_sellected.size(); t++)
				{
					if (m_sellected[t] == point)
					{
						
						//ROTATE
				
						//SET X
						if (input->GetKeyboardState(DIK_NUMPAD7))
						{
							poss = poss + XMFLOAT3(1, 0, 0)*m_force;
						}
						if (input->GetKeyboardState(DIK_NUMPAD4))
						{
							poss = poss + XMFLOAT3(-1, 0, 0)*m_force;
						}

						//SET Y
						if (input->GetKeyboardState(DIK_NUMPAD8))
						{
							poss = poss + XMFLOAT3(0, 1, 0)*m_force;
						}
						if (input->GetKeyboardState(DIK_NUMPAD5))
						{
							poss = poss + XMFLOAT3(0, -1, 0)*m_force;
						}

						//SET Z
						if (input->GetKeyboardState(DIK_NUMPAD9))
						{
							poss = poss + XMFLOAT3(0, 0, 1)*m_force;
						}
						if (input->GetKeyboardState(DIK_NUMPAD6))
						{
							poss = poss + XMFLOAT3(0, 0, -1)*m_force;
						}
						obj->m_controlPoints[point]->m_force = poss;
					}
				}
			}
			for (unsigned int x = 0; x < m_particles.size(); x++)
			{
				m_particles[x]->ColourFadePos(dx, 3.5f, XMFLOAT4(255, 255, 255, 0), dt);
			}

			for (unsigned int x = 0; x < m_sellected.size(); x++)
			{
				m_particles[m_sellected[x]]->ColourFadePos(dx, 2.5f, XMFLOAT4(255, 0, 0, 0), dt);
			}

			for (unsigned int x = 0; x < m_static.size(); x++)
			{
				m_particles[m_static[x]]->ColourFadePos(dx, 2.5f, XMFLOAT4(170, 170, 0, 0), dt);
			}
		}
	}
#pragma endregion


#pragma region InputChange

	if (input->GetKeyboardState(DIK_1) && input->GetKeyboardPrevState(DIK_1))
	{
		m_deformType = 0;
	}

	if (input->GetKeyboardState(DIK_2) && input->GetKeyboardPrevState(DIK_2))
	{
		m_deformType = 1;
	}
	if (input->GetKeyboardState(DIK_3) && input->GetKeyboardPrevState(DIK_3))
	{
		m_deformType = 2;
	}
	if (input->GetKeyboardState(DIK_4) && input->GetKeyboardPrevState(DIK_4))
	{
		m_deformType = 3;
	}

#pragma endregion

	//Switch between different implementations
	switch (m_deformType)
	{

#pragma region BASIC
	case 0:
	{
			  m_deformationType = "BASIC";

			  for each (MeshObject* obj in m_deformObjects)
			  {
				  CalcualteDeformedMeshSum(obj);

				  //Calcualte Deformed shape 
				  CalculateDeformedCom(obj);
				  //Calcualte relative psition of defomred points.
				  Calculate_p(obj);

				  CalcualteApqMatrix(obj);

				  //Calcualte RotationMatrix
				  XMMATRIX RotMat = ComputeOptimumRotation(XMLoadFloat4x4(&obj->m_Apq));

				  for (unsigned int point = 0; point < obj->m_controlPoints.size(); point++)
				  {
					  for (int x = 0; x < m_static.size(); x++)
					  {
						  if (point == m_static[x])
						  {
							  if (point < obj->m_controlPoints.size())
							  {
								  point++;
						  }
						  }
					  }
							  XMFLOAT3* q = &obj->m_controlPoints[point]->m_originalPos;
							  //ROTATE Point by a vector

							  XMFLOAT3 goalPosition = rotateVect(MatrixToXMFLOAT3X3(RotMat), *q);

							  //Offset to deformed position.
							  goalPosition = goalPosition + obj->m_originalCOM;
							  //Assign New goalposition  for a point
							  obj->m_controlPoints[point]->m_goalPosition = goalPosition;

							  //Update point //veocity//position..

							  PointUpdate(obj->m_controlPoints[point], dt);

							  //update model vertecies
							  obj->object->GetModel().verticesPosNor[point].position = obj->m_controlPoints[point]->m_goalPosition;

							 m_particles[point]->Update(dt);
							 m_particles[point]->SetPosition(obj->m_controlPoints[point]->m_currentPos + obj->object->GetPosition());
						 
				  }
			  }
			  break;
	}

#pragma endregion

#pragma region Linear

	case 1:
	{
			  m_deformationType = "LINEAR";
			  //LINEAR DEFORMATION

			  for each (MeshObject* obj in m_deformObjects)
			  {
				  CalcualteDeformedMeshSum(obj);

				  //calcualte center of mass for a deformed shape
				  CalculateDeformedCom(obj);
				  //calcualte control point position relative to the center of the deformed shape
				  Calculate_p(obj);
				  //transformation matrix
				  CalcualteApqMatrix(obj);
				  
				  //linear transformation matrix
				  XMMATRIX A = XMLoadFloat4x4(&obj->m_Apq) * XMLoadFloat4x4(&obj->m_Aqq);
				  //Extract Optimal Rotation from the transformation matrix of the deformed shape.
				  XMMATRIX RotMat = ComputeOptimumRotation(XMLoadFloat4x4(&obj->m_Apq));
				  //Linear TransformationMatrix
				  XMMATRIX LinearTransform = volumeNormalize(A);
				  XMMATRIX transform;

				  for (unsigned int x = 0; x < 3; x++)
				  {
					  for (unsigned int y = 0; y < 3; y++)
					  {
						  LinearTransform(x, y) *= m_deformObjects[m_deformObjects.size() - 1]->m_beta;
						  RotMat(x, y) *= (1 - m_deformObjects[m_deformObjects.size() - 1]->m_beta);
						  transform(x, y) = LinearTransform(x, y) + RotMat(x, y);
					  }
				  }

				  for (unsigned int point = 0; point < obj->m_controlPoints.size(); point++)
				  {
					  for (int x = 0; x < m_static.size(); x++)
					  {
						  if (point == m_static[x])
						  {
							  if (point < obj->m_controlPoints.size())
							  {
								  point++;
							  }
						  }
					  }
							  //deformed relative position.
							  XMFLOAT3* q = &obj->m_controlPoints[point]->m_originalPos;
							  //rotate the shape by the extracted rotation matrix.
							  XMFLOAT3 goalPosition = rotateVect(MatrixToXMFLOAT3X3(transform), *q);
							  //Set the shape to the original Position.
							  goalPosition = goalPosition + obj->m_originalCOM;
							  obj->m_controlPoints[point]->m_goalPosition = goalPosition;
							  ////set the new control point position
							  PointUpdate(obj->m_controlPoints[point], dt);
							  //update model vertecies
							  obj->object->GetModel().verticesPosNor[point].position = obj->m_controlPoints[point]->m_currentPos;


							  //Update point
							  m_particles[point]->Update(dt);
							  //set visual point positions
							  m_particles[point]->SetPosition(obj->m_controlPoints[point]->m_currentPos + obj->object->GetPosition());
						  
					  
				  }
			  }
			  break;
	}
#pragma endregion

#pragma region Quadratic

	case 2:
	{
			  m_deformationType = "QUAD";
			  //QUADRATIC DEFORMATION
			  for each (MeshObject* obj in m_deformObjects)
			  {
			  CalcualteDeformedMeshSum(obj);

			  CalcualteApqMatrix(obj);
			
				  Calculate_p(obj);
				  Calculate_q_tilde(obj);
				  CalcualteAqqMatrix_Tilde(obj);
				  CalcualteApqMatrix_Tilde(obj);

				  MatrixXd AA(9, 9);
				  AA = obj->m_Apq_tilde * obj->m_Aqq_tilde;

				  XMFLOAT4X4 ARot;
				  {
					  for (unsigned int row = 0; row < 3; row++) {
						  for (unsigned int col = 0; col < 3; col++) {
							  ARot(row, col) = AA(row, col);
						  }
					  }
				  }

				  XMMATRIX RotMat = ComputeOptimumRotation(XMLoadFloat4x4(&ARot));
				  MatrixXd Rt(3, 9);
				  for (unsigned int row = 0; row < 3; row++) {
					  for (unsigned int col = 0; col < 3; col++) {
						  Rt(row, col) = RotMat(row, col);
					  }
				  }


					  XMMATRIX LinearTransform = volumeNormalize(XMLoadFloat4x4(&ARot));
					  {
						  for (unsigned int row = 0; row < 3; row++) {
							  for (unsigned int col = 0; col < 3; col++) {
								  AA(row, col) = LinearTransform(row, col);
							  }
						  }
					  }
				  

				  MatrixXd Ttilde(3, 9);
				  Ttilde = m_deformObjects[m_deformObjects.size() - 1]->m_beta* AA + (1 - m_deformObjects[m_deformObjects.size() - 1]->m_beta) * Rt;


				  for (unsigned int point = 0; point < obj->m_controlPoints.size(); point++)
				  {
					  for (int x = 0; x < m_static.size(); x++)
					  {
						  if (point == m_static[x])
						  {
							  if (point < obj->m_controlPoints.size())
							  {
								  point++;
							  }
						  }
					  }
							  MatrixXd qcol(9, 1);
							  qcol <<
								  obj->m_controlPoints[point]->q_quad[0],
								  obj->m_controlPoints[point]->q_quad[1],
								  obj->m_controlPoints[point]->q_quad[2],

								  obj->m_controlPoints[point]->q_quad[3],
								  obj->m_controlPoints[point]->q_quad[4],
								  obj->m_controlPoints[point]->q_quad[5],

								  obj->m_controlPoints[point]->q_quad[6],
								  obj->m_controlPoints[point]->q_quad[7],
								  obj->m_controlPoints[point]->q_quad[8];


							  MatrixXd g2 = Ttilde * qcol;
							  XMFLOAT3 goalPosition = XMFLOAT3((float)g2(0, 0), (float)g2(1, 0), (float)g2(2, 0));
							  //Offset to deformed position.
							  goalPosition = goalPosition + obj->m_originalCOM;

							  //Assign New goalposition 
							  obj->m_controlPoints[point]->m_goalPosition = goalPosition;

							  //Update point
							  m_particles[point]->Update(dt);
							  PointUpdate(obj->m_controlPoints[point], dt);
							  m_particles[point]->SetPosition(obj->m_controlPoints[point]->m_currentPos + obj->object->GetPosition());
							  //update model vertecies
							  obj->object->GetModel().verticesPosNor[point].position = obj->m_controlPoints[point]->m_currentPos;
						  
					  
				  }
			  }
			  break;
	}

#pragma endregion

#pragma region ClusteringTry
	case 3:
	{
			  m_deformationType = "CLUSTERING";
			  //LINEAR DEFORMATION

			  for (unsigned int x = 0; x < m_clusterList.size(); x++)
			  {

				  CalculateDeformedComCluster(m_clusterList[x]);
				  Calculate_pCluster(m_clusterList[x]);
				  CalcualteApqMatrixCluster(m_clusterList[x]);

				  //Translation Matrix
				  XMMATRIX A = XMLoadFloat4x4(&m_clusterList[x].m_Apq) * XMLoadFloat4x4(&m_clusterList[x].m_Aqq);
				  //Calculate Optimal Rotation.
				  XMMATRIX RotMat = ComputeOptimumRotation(XMLoadFloat4x4(&m_clusterList[x].m_Apq));
				  //Linear TransformationMatrix
				  XMMATRIX LinearTransform = volumeNormalize(A);
				  XMMATRIX transform;
				  {
					  for (unsigned int x = 0; x < 3; x++)
					  {
						  for (unsigned int y = 0; y < 3; y++)
						  {
							  LinearTransform(x, y) *= m_deformObjects[m_deformObjects.size() - 1]->m_beta;
							  RotMat(x, y) *= (1 - m_deformObjects[m_deformObjects.size() - 1]->m_beta);
							  transform(x, y) = LinearTransform(x, y) + RotMat(x, y);
						  }
					  }
				  }

				  for (unsigned int ii = 0; ii < m_clusterList[x].m_vertecies.size(); ii++)
				  {
					  MeshObject* GameObject = m_deformObjects[m_deformObjects.size() - 1];
					  //deformed relative position.
					  XMFLOAT3* q = &GameObject->m_controlPoints[m_clusterList[x].m_vertecies[ii]]->m_relativePosOriginal;
					  //rotate the shape by the extracted rotation matrix.
					  XMFLOAT3 goalPosition = rotateVect(MatrixToXMFLOAT3X3(transform), *q);
					  //Set the shape to the original Position.
					  goalPosition = goalPosition + GameObject->m_originalCOM;
					  GameObject->m_controlPoints[m_clusterList[x].m_vertecies[ii]]->m_goalPosition = goalPosition;
					  //set the new control point position
					  PointUpdate(GameObject->m_controlPoints[m_clusterList[x].m_vertecies[ii]], dt);
					  //update model vertecies
					  GameObject->object->GetModel().verticesPosNor[m_clusterList[x].m_vertecies[ii]].position = GameObject->m_controlPoints[m_clusterList[x].m_vertecies[ii]]->m_goalPosition;


					  //Update point
					  m_particles[m_clusterList[x].m_vertecies[ii]]->Update(dt);
					  //set visual point positions
					  m_particles[m_clusterList[x].m_vertecies[ii]]->SetPosition(GameObject->m_controlPoints[m_clusterList[x].m_vertecies[ii]]->m_currentPos + GameObject->object->GetPosition());
				 }
			  }
			  for (unsigned int x = 0; x < m_clusterLineList.size(); x++)
			  {
				  //m_clusterLineList[x]->setPosition(p_box->GetPosition());
				  m_clusterLineList[x]->Update(dt);
			  }
			  break;
	}
#pragma endregion

	}

	//Remap GameObject Buffers
	for each (MeshObject* obj in m_deformObjects)
	{
		BufferRemap(obj, dx);
	}
}

void MeshDeformation::Render(DxGraphics* dx, Camera& mCam, LightManager & lightManager)
{
	TwDraw();
	for each(Particle* p in m_particles)
	{
		p->Render(dx, mCam);
	}

	if (m_deformationType == "CLUSTERING")
	{
		for (unsigned int x = 0; x < m_clusterLineList.size(); x++)
		{
			m_clusterLineList[x]->Render(dx, &mCam);
		}
	}
}


void MeshDeformation::InitialCalculations(MeshObject* obj)
{
	//calcualte original center of mass of the non deformed shape
	CalcualteOriginalCom(obj);
	//calcualte relative aprticle positions
	Calculate_q(obj);
	//calcualte scaling matrix
	CalcualteAqqMatrix(obj);

	//Mesh CheckSum
	CalcualteOriginalMeshSum(obj);

	//calc volume
	obj->m_originalVolume = calcVolumeOriginal();
}

//Calcualte position of each control point relative to the center of the shape (p - Deformed Shape, q - Original Shape)
void MeshDeformation::Calculate_p(MeshObject* object)
{
	//Update relative point positions of Deformed shape
	for (unsigned int point = 0; point < object->m_controlPoints.size(); point++)
	{
		object->m_controlPoints[point]->m_relativePosDeformed = object->m_controlPoints[point]->m_currentPos - object->m_deformedCOM;
	}
}
void MeshDeformation::Calculate_q(MeshObject* object)
{
	//Update relative point positions of Original shape
	for (unsigned int point = 0; point < object->m_controlPoints.size(); point++)
	{
		object->m_controlPoints[point]->m_relativePosDeformed = object->m_controlPoints[point]->m_originalPos - object->m_deformedCOM;
	}
}

void MeshDeformation::CalcualteApqMatrix(MeshObject* object)
{
	//Calcualte Apq Matrix ( rotation information )
	XMMATRIX Apq;
	Apq = XMMatrixIdentity();
	for (unsigned int point = 0; point < object->m_controlPoints.size(); point++)
	{
		float* m = &object->m_controlPoints[point]->m_mass;
		XMFLOAT3* q = &object->m_controlPoints[point]->m_originalPos;
		XMFLOAT3* p = &object->m_controlPoints[point]->m_relativePosDeformed;
		Apq(0, 0) += *m * p->x * q->x;
		Apq(0, 1) += *m * p->y * q->x;
		Apq(0, 2) += *m * p->z * q->x;

		Apq(1, 0) += *m * p->x * q->y;
		Apq(1, 1) += *m * p->y * q->y;
		Apq(1, 2) += *m * p->z * q->y;

		Apq(2, 0) += *m * p->x * q->z;
		Apq(2, 1) += *m * p->y * q->z;
		Apq(2, 2) += *m * p->z * q->z;
	}
	//m_Apq = Apq;
	XMStoreFloat4x4(&object->m_Apq,Apq);
}
void MeshDeformation::CalcualteAqqMatrix(MeshObject* object)
{
	XMMATRIX AqqInverse;
	AqqInverse = XMMatrixIdentity();
	for (unsigned int point = 0; point < object->m_controlPoints.size(); point++)
	{
		float* m = &object->m_controlPoints[point]->m_mass;
		XMFLOAT3* q = &object->m_controlPoints[point]->m_originalPos;
		AqqInverse(0, 0) += *m * q->x * q->x;
		AqqInverse(0, 1) += *m * q->y * q->x;
		AqqInverse(0, 2) += *m * q->z * q->x;

		AqqInverse(1, 0) += *m * q->x * q->y;
		AqqInverse(1, 1) += *m * q->y * q->y;
		AqqInverse(1, 2) += *m * q->z * q->y;

		AqqInverse(2, 0) += *m * q->x * q->z;
		AqqInverse(2, 1) += *m * q->y * q->z;
		AqqInverse(2, 2) += *m * q->z * q->z;
	}

	//needs inverse
	XMVECTOR det;
	AqqInverse = XMMatrixInverse(&det, AqqInverse);
	//object->m_Aqq = AqqInverse;
	XMStoreFloat4x4(&object->m_Aqq, AqqInverse);
}

//Calcualte CenterOfMass of Original and Deformed shapes.
void MeshDeformation::CalcualteOriginalCom(MeshObject* object)
{
	
	//Calcualte center of mass for original shape
	XMFLOAT3 positionSum = XMFLOAT3(0, 0, 0);
	float massSum = 1;

	for (unsigned int point = 0; point < object->m_controlPoints.size(); point++)
	{
		positionSum = positionSum + (object->m_controlPoints[point]->m_originalPos * object->m_controlPoints[point]->m_mass);
		massSum += object->m_controlPoints[point]->m_mass;
	}
	object->m_originalCOM = positionSum / massSum;
}
void MeshDeformation::CalculateDeformedCom(MeshObject* object)
{
	//Calcualte center of mass for deformed shape
	XMFLOAT3 positionSum = XMFLOAT3(0, 0, 0);
	float massSum = 1;

	for (unsigned int point = 0; point < object->m_controlPoints.size(); point++)
	{
		positionSum = positionSum + (object->m_controlPoints[point]->m_currentPos * object->m_controlPoints[point]->m_mass);
		massSum += object->m_controlPoints[point]->m_mass;
	}
	object->m_deformedCOM = positionSum / massSum;
}

//Quadratic term calcualtions
void MeshDeformation::Calculate_q_tilde(MeshObject* object)
{
	//Update relative point positions of Original shape
	for (unsigned int point = 0; point < object->m_controlPoints.size(); point++)
	{
		
		object->m_controlPoints[point]->q_quad[0] = object->m_controlPoints[point]->m_originalPos.x;
		object->m_controlPoints[point]->q_quad[1] = object->m_controlPoints[point]->m_originalPos.y;
		object->m_controlPoints[point]->q_quad[2] = object->m_controlPoints[point]->m_originalPos.z;

		object->m_controlPoints[point]->q_quad[3] = object->m_controlPoints[point]->m_originalPos.x * object->m_controlPoints[point]->m_originalPos.x;
		object->m_controlPoints[point]->q_quad[4] = object->m_controlPoints[point]->m_originalPos.y * object->m_controlPoints[point]->m_originalPos.y;
		object->m_controlPoints[point]->q_quad[5] = object->m_controlPoints[point]->m_originalPos.z * object->m_controlPoints[point]->m_originalPos.z;

		object->m_controlPoints[point]->q_quad[6] = object->m_controlPoints[point]->m_originalPos.x * object->m_controlPoints[point]->m_originalPos.y;
		object->m_controlPoints[point]->q_quad[7] = object->m_controlPoints[point]->m_originalPos.y * object->m_controlPoints[point]->m_originalPos.z;
		object->m_controlPoints[point]->q_quad[8] = object->m_controlPoints[point]->m_originalPos.z * object->m_controlPoints[point]->m_originalPos.x;
	}
}
void MeshDeformation::CalcualteAqqMatrix_Tilde(MeshObject* object)
{
	MatrixXd AqqInverse(9, 9);
	AqqInverse = MatrixXd::Identity(9, 9);

	for (unsigned int point = 0; point < object->m_controlPoints.size(); point++)
	{
		float m = object->m_controlPoints[point]->m_mass;
		XMFLOAT3* q = &object->m_controlPoints[point]->m_originalPos;
		MatrixXd qT(1, 9);
		qT << q->x,
			q->y,
			q->z,
			q->x*q->x,
			q->y*q->y,
			q->z*q->z,
			q->x * q->y,
			q->y * q->z,
			q->z*q->x;

		MatrixXd qnt(9, 1);
		qnt = qT;
		qT.transposeInPlace();
		MatrixXd res = qT * qnt;
		AqqInverse += res;
	}

	//needs inverse
	//	XMVECTOR det;
	object->m_Aqq_tilde = AqqInverse.inverse();
}
void MeshDeformation::CalcualteApqMatrix_Tilde(MeshObject* object)
{
	MatrixXd Apq(3, 9);
	Apq = MatrixXd::Identity(3, 9);

	for (unsigned int point = 0; point < object->m_controlPoints.size(); point++)
	{
		float m = object->m_controlPoints[point]->m_mass;
		XMFLOAT3* p = &object->m_controlPoints[point]->m_relativePosDeformed;
		MatrixXd pp(3, 1);
		pp << p->x, p->y, p->z;

		XMFLOAT3* q = &object->m_controlPoints[point]->m_originalPos;
		MatrixXd qT(1, 9);
		qT << q->x,
			q->y,
			q->z,
			q->x*q->x,
			q->y*q->y,
			q->z*q->z,
			q->x * q->y,
			q->y * q->z,
			q->z*q->x;

		MatrixXd res = pp * qT;
		Apq += res;
	}

	//needs inverse
	//	XMVECTOR det;
	object->m_Apq_tilde = Apq;
}

void MeshDeformation::PointUpdate(ControlPoint* point, float dt)
{
	XMFLOAT3 velocity = point->velocity;
	velocity = velocity + ((((point->m_goalPosition - point->m_currentPos) * m_deformObjects[m_deformObjects.size() - 1]->m_alpha) / dt)) + (point->m_goalPosition - point->m_force*dt / point->m_mass);
	
	XMFLOAT3 pos = point->m_currentPos;
	XMFLOAT3 t = velocity * dt;
	XMFLOAT3 newPos = pos + t;
	point->m_currentPos = newPos;
}


//CLUSTERING
void MeshDeformation::Calculate_qCluster(Cluster& _cluster)
{
	MeshObject* GameObject = m_deformObjects[m_deformObjects.size() - 1];
	//Update relative point positions of Original shape
	for (unsigned int x = 0; x < _cluster.m_vertecies.size(); x++)
	{
		GameObject->m_controlPoints[_cluster.m_vertecies[x]]->m_relativePosDeformed 
			= GameObject->m_controlPoints[_cluster.m_vertecies[x]]->m_originalPos - _cluster.m_originalCenterOfMass;
	}
}

void MeshDeformation::CalcualteOriginalComCluster(Cluster& _cluster)
{
	//Calcualte center of mass for original shape
	XMFLOAT3 positionSum = XMFLOAT3(0, 0, 0);
	float massSum = 1;
	MeshObject* GameObject = m_deformObjects[m_deformObjects.size() - 1];

	for (unsigned int pId = 0; pId < _cluster.m_vertecies.size(); pId++)
	{
		positionSum = positionSum + (GameObject->m_controlPoints[_cluster.m_vertecies[pId]]->m_originalPos * 1);
		massSum += 1;
	}
	_cluster.m_originalCenterOfMass = positionSum / massSum;
}

void MeshDeformation::CalcualteAqqMatrixCluster(Cluster& _cluster)
{
	MeshObject* GameObject = m_deformObjects[m_deformObjects.size() - 1];
	XMMATRIX AqqInverse;
	AqqInverse = XMMatrixIdentity();
	for (unsigned int x = 0; x < _cluster.m_vertecies.size(); x++)
	{
		float m = GameObject->m_controlPoints[_cluster.m_vertecies[x]]->m_mass;
		XMFLOAT3* q = &GameObject->m_controlPoints[_cluster.m_vertecies[x]]->m_relativePosOriginal;
		AqqInverse(0, 0) += m * q->x * q->x;
		AqqInverse(0, 1) += m * q->y * q->x;
		AqqInverse(0, 2) += m * q->z * q->x;

		AqqInverse(1, 0) += m * q->x * q->y;
		AqqInverse(1, 1) += m * q->y * q->y;
		AqqInverse(1, 2) += m * q->z * q->y;

		AqqInverse(2, 0) += m * q->x * q->z;
		AqqInverse(2, 1) += m * q->y * q->z;
		AqqInverse(2, 2) += m * q->z * q->z;
	}

	//needs inverse
	XMVECTOR det;
	XMMATRIX m_Aqq = XMMatrixInverse(&det, AqqInverse);
	XMStoreFloat4x4(&_cluster.m_Aqq, m_Aqq);
}

void MeshDeformation::Calculate_pCluster(Cluster& _cluster)
{
	MeshObject* GameObject = m_deformObjects[m_deformObjects.size() - 1];
	//Update relative point positions of Deformed shape
	for (unsigned int x = 0; x < _cluster.m_vertecies.size(); x++)
	{
		GameObject->m_controlPoints[_cluster.m_vertecies[x]]->m_relativePosDeformed = 
			GameObject->m_controlPoints[_cluster.m_vertecies[x]]->m_currentPos - _cluster.m_deformedCenterOfMass;
	}
}

void MeshDeformation::CalculateDeformedComCluster(Cluster& _cluster)
{
	//Calcualte center of mass for deformed shape
	XMFLOAT3 positionSum = XMFLOAT3(0, 0, 0);
	float massSum = 1;
	MeshObject* GameObject = m_deformObjects[m_deformObjects.size() - 1];

	for (unsigned int x = 0; x < _cluster.m_vertecies.size(); x++)
	{
		positionSum = positionSum + (GameObject->m_controlPoints[_cluster.m_vertecies[x]]->m_currentPos * 1);
		massSum += 1;
	}
	_cluster.m_deformedCenterOfMass = positionSum / massSum;
}

void MeshDeformation::CalcualteApqMatrixCluster(Cluster& _cluster)
{
	//Calcualte Apq Matrix ( rotation information )
	XMMATRIX Apq;
	Apq = XMMatrixIdentity();
	MeshObject* GameObject = m_deformObjects[m_deformObjects.size() - 1];

	for (unsigned int x = 0; x < _cluster.m_vertecies.size(); x++)
	{
		float m = m_particles[_cluster.m_vertecies[x]]->m_mass;
		XMFLOAT3* q = &GameObject->m_controlPoints[_cluster.m_vertecies[x]]->m_relativePosOriginal;
		XMFLOAT3* p = &GameObject->m_controlPoints[_cluster.m_vertecies[x]]->m_relativePosDeformed;
		Apq(0, 0) += m * p->x * q->x;
		Apq(0, 1) += m * p->y * q->x;
		Apq(0, 2) += m * p->z * q->x;

		Apq(1, 0) += m * p->x * q->y;
		Apq(1, 1) += m * p->y * q->y;
		Apq(1, 2) += m * p->z * q->y;

		Apq(2, 0) += m * p->x * q->z;
		Apq(2, 1) += m * p->y * q->z;
		Apq(2, 2) += m * p->z * q->z;
	}
	XMStoreFloat4x4(&_cluster.m_Apq, Apq);
}

void MeshDeformation::ClusterCalutate(DxGraphics* dx, ResourceManager & resource)
{
	vector<int> ClusterTop, ClusterBottom;

	//Divide vertex buddef by clustnum,separate 
	XMFLOAT3 numOfClusters = XMFLOAT3(1, 2, 1);
	//create 2 clusters
	m_clusterList.push_back(Cluster());
	m_clusterList.push_back(Cluster());

	for (unsigned int x = 0; x < m_deformObjects[m_deformObjects.size() - 1]->object->GetModel().verticesPosNor.size(); x++)
	{
		if (m_deformObjects[m_deformObjects.size() - 1]->object->GetModel().verticesPosNor[x].position.y < 0.5)
		{
			//add indexes of clusters
			ClusterBottom.push_back(x);
		}

		if (m_deformObjects[m_deformObjects.size() - 1]->object->GetModel().verticesPosNor[x].position.y >-0.5)
		{
			//add indexes of clusters
			ClusterTop.push_back(x);
		}
	}

	//ASSIGN THE CLUSTERS
	m_clusterList[1].m_vertecies = ClusterTop;
	m_clusterList[0].m_vertecies = ClusterBottom;

	for (unsigned int x = 0; x < m_clusterList.size(); x++)
	{

		//calcualte original center of mass of the non deformed shape
		CalcualteOriginalComCluster(m_clusterList[x]);
		//calcualte relative aprticle positions
		Calculate_qCluster(m_clusterList[x]);
		//calcualte scaling matrix
		CalcualteAqqMatrixCluster(m_clusterList[x]);
	}

	for (unsigned int xx = 0; xx < m_clusterList.size(); xx++)
	{
		XMFLOAT3 min = XMFLOAT3(0, 0, 0), max = XMFLOAT3(0, 0, 0);
		for (unsigned int y = 0; y < m_clusterList[xx].m_vertecies.size(); y++)
		{
			if (m_particles[m_clusterList[xx].m_vertecies[y]]->GetPosition().x < min.x)
			{
				min.x = m_particles[m_clusterList[xx].m_vertecies[y]]->GetPosition().x;
			}
			if (m_particles[m_clusterList[xx].m_vertecies[y]]->GetPosition().y < min.y)
			{
				min.y = m_particles[m_clusterList[xx].m_vertecies[y]]->GetPosition().y;
			}
			if (m_particles[m_clusterList[xx].m_vertecies[y]]->GetPosition().z < min.z)
			{
				min.z = m_particles[m_clusterList[xx].m_vertecies[y]]->GetPosition().z;
			}

			if (m_particles[m_clusterList[xx].m_vertecies[y]]->GetPosition().x > max.x)
			{
				max.x = m_particles[m_clusterList[xx].m_vertecies[y]]->GetPosition().x;
			}
			if (m_particles[m_clusterList[xx].m_vertecies[y]]->GetPosition().y > max.y)
			{
				max.y = m_particles[m_clusterList[xx].m_vertecies[y]]->GetPosition().y;
			}
			if (m_particles[m_clusterList[xx].m_vertecies[y]]->GetPosition().z > max.z)
			{
				max.z = m_particles[m_clusterList[xx].m_vertecies[y]]->GetPosition().z;
			}
		}

		m_clusterLineList.push_back(new Line());
		m_clusterLineList[m_clusterLineList.size() - 1]->LoadContent(dx, resource);

		if (xx == 0)max.y += 0.5;
		if (xx == 1)min.y -= 0.5;

		m_clusterLineList[m_clusterLineList.size() - 1]->SetSize(min, max);
	}
}


//MESH SUM CALACULATION
void MeshDeformation::CalcualteOriginalMeshSum(MeshObject* object)
{
	m_originalMeshSum = 0;
	for (unsigned int x = 0; x < object->m_controlPoints.size(); x++)
	{
		m_originalMeshSum = (object->m_controlPoints[x]->m_originalPos.x + object->m_controlPoints[x]->m_originalPos.y + object->m_controlPoints[x]->m_originalPos.z);
	}
	m_originalMeshSum = m_originalMeshSum / object->m_controlPoints.size();
}

void MeshDeformation::CalcualteDeformedMeshSum(MeshObject* object)
{
	m_deformedMeshSum = 0;
	for (unsigned int x = 0; x < object->m_controlPoints.size(); x++)
	{
		
		m_deformedMeshSum = (object->m_controlPoints[x]->m_goalPosition.x + object->m_controlPoints[x]->m_goalPosition.y + object->m_controlPoints[x]->m_goalPosition.z);
	}
	m_deformedMeshSum = m_deformedMeshSum / object->m_controlPoints.size();
}

void MeshDeformation::BufferRemap(MeshObject* object, DxGraphics *dx)
{
	//remap buffer
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	//	Disable GPU access to the vertex buffer data.
	dx->GetImmediateContext()->Map(object->object->GetVertexBuffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	//	Update the vertex buffer here.
	memcpy(mappedResource.pData, &object->object->GetModel().verticesPosNor[0], sizeof(VertexPosNor)* object->object->GetModel().verticesPosNor.size());
	//	Reenable GPU access to the vertex buffer data.
	dx->GetImmediateContext()->Unmap(object->object->GetVertexBuffer(), 0);
}

float MeshDeformation::calcVolumeOriginal()
{	
		float volume = 0;

		for (int point = 0; point < m_deformObjects[m_deformObjects.size() - 1]->m_controlPoints.size() / 3; point += 3)
		{
			XMFLOAT3 p1 = m_deformObjects[m_deformObjects.size() - 1]->m_controlPoints[point + 0]->m_originalPos;
			XMFLOAT3 p2 = m_deformObjects[m_deformObjects.size() - 1]->m_controlPoints[point + 1]->m_originalPos;
			XMFLOAT3 p3 = m_deformObjects[m_deformObjects.size() - 1]->m_controlPoints[point + 2]->m_originalPos;

			float v321 = p3.x * p2.y * p1.z;
			float v231 = p2.x * p3.y * p1.z;
			float v312 = p3.x * p1.y * p2.z;
			float v132 = p1.x * p3.y * p2.z;
			float v213 = p2.x * p1.y * p3.z;
			float v123 = p1.x * p2.y * p3.z;

			volume += (1.0f / 6.0f) * (-v321 + v231 + v312 - v132 - v213 + v123);
		}

		return abs(volume);
}

float MeshDeformation::calcVolumeDeformed()
{
	float volume = 0;

	for (int point = 0; point < m_deformObjects[m_deformObjects.size() - 1]->m_controlPoints.size() / 3; point += 3)
	{
		XMFLOAT3 p1 = m_deformObjects[m_deformObjects.size() - 1]->m_controlPoints[point + 0]->m_goalPosition;
		XMFLOAT3 p2 = m_deformObjects[m_deformObjects.size() - 1]->m_controlPoints[point + 1]->m_goalPosition;
		XMFLOAT3 p3 = m_deformObjects[m_deformObjects.size() - 1]->m_controlPoints[point + 2]->m_goalPosition;

		float v321 = p3.x * p2.y * p1.z;
		float v231 = p2.x * p3.y * p1.z;
		float v312 = p3.x * p1.y * p2.z;
		float v132 = p1.x * p3.y * p2.z;
		float v213 = p2.x * p1.y * p3.z;
		float v123 = p1.x * p2.y * p3.z;

		volume += (1.0f / 6.0f) * (-v321 + v231 + v312 - v132 - v213 + v123);
	}

	return abs(volume);
}


//SHADER

//--------------------------------------------------------------------------------------
// Compile and create the CS
//--------------------------------------------------------------------------------------
HRESULT MeshDeformation::CreateComputeShader(LPCSTR pSrcFile, LPCSTR pFunctionName,
	ID3D11Device* pDevice, ID3D11ComputeShader** ppShaderOut)
{
	HRESULT hr;

	DWORD dwShaderFlags = 0;

	// We generally prefer to use the higher CS shader profile when possible as CS 5.0 is better performance on 11-class hardware
	LPCSTR pProfile = (pDevice->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0) ? "cs_5_0" : "cs_4_0";

	ID3DBlob* pErrorBlob = NULL;
	ID3DBlob* pBlob = NULL;
	hr = D3DX11CompileFromFile(pSrcFile, 0, 0, pFunctionName, pProfile,
		dwShaderFlags, NULL, NULL, &pBlob, &pErrorBlob, NULL);


	if (pErrorBlob != 0)
	{
		MessageBoxA(NULL, (char*)pErrorBlob->GetBufferPointer(), 0, 0);
		pErrorBlob->Release();
		pErrorBlob = 0;
	}

	//If the compile failed completely then return a DXTRACE msg to link back to this function call
	if (FAILED(hr))
	{
		DXTRACE_MSG(__FILE__, (DWORD)__LINE__, hResult, "D3DX11CompileFromFile", true);
		return false;
	}


	hr = pDevice->CreateComputeShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, ppShaderOut);

#if defined(DEBUG) || defined(PROFILE)
	if (*ppShaderOut)
		(*ppShaderOut)->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(pFunctionName), pFunctionName);
#endif

	//	SAFE_RELEASE(pErrorBlob);
	//	SAFE_RELEASE(pBlob);

	return hr;
}

//--------------------------------------------------------------------------------------
// Create Structured Buffer
//--------------------------------------------------------------------------------------
HRESULT MeshDeformation::CreateStructuredBuffer(ID3D11Device* pDevice, UINT uElementSize, UINT uCount, VOID* pInitData, ID3D11Buffer** ppBufOut)
{
	*ppBufOut = NULL;

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	desc.ByteWidth = uElementSize * uCount;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = uElementSize;

	if (pInitData)
	{
		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = pInitData;
		return pDevice->CreateBuffer(&desc, &InitData, ppBufOut);
	}
	else
		return pDevice->CreateBuffer(&desc, NULL, ppBufOut);
}

//--------------------------------------------------------------------------------------
// Create Raw Buffer
//--------------------------------------------------------------------------------------
HRESULT MeshDeformation::CreateRawBuffer(ID3D11Device* pDevice, UINT uSize, VOID* pInitData, ID3D11Buffer** ppBufOut)
{
	*ppBufOut = NULL;

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_INDEX_BUFFER | D3D11_BIND_VERTEX_BUFFER;
	desc.ByteWidth = uSize;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;

	if (pInitData)
	{
		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = pInitData;
		return pDevice->CreateBuffer(&desc, &InitData, ppBufOut);
	}
	else
		return pDevice->CreateBuffer(&desc, NULL, ppBufOut);
}

//--------------------------------------------------------------------------------------
// Create Shader Resource View for Structured or Raw Buffers
//--------------------------------------------------------------------------------------
HRESULT MeshDeformation::CreateBufferSRV(ID3D11Device* pDevice, ID3D11Buffer* pBuffer, ID3D11ShaderResourceView** ppSRVOut)
{
	D3D11_BUFFER_DESC descBuf;
	ZeroMemory(&descBuf, sizeof(descBuf));
	pBuffer->GetDesc(&descBuf);

	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	desc.BufferEx.FirstElement = 0;

	if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
	{
		// This is a Raw Buffer

		desc.Format = DXGI_FORMAT_R32_TYPELESS;
		desc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
		desc.BufferEx.NumElements = descBuf.ByteWidth / 4;
	}
	else
	if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
	{
		// This is a Structured Buffer

		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.BufferEx.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;
	}
	else
	{
		return E_INVALIDARG;
	}

	return pDevice->CreateShaderResourceView(pBuffer, &desc, ppSRVOut);
}

//--------------------------------------------------------------------------------------
// Create Unordered Access View for Structured or Raw Buffers
//-------------------------------------------------------------------------------------- 
HRESULT MeshDeformation::CreateBufferUAV(ID3D11Device* pDevice, ID3D11Buffer* pBuffer, ID3D11UnorderedAccessView** ppUAVOut)
{
	D3D11_BUFFER_DESC descBuf;
	ZeroMemory(&descBuf, sizeof(descBuf));
	pBuffer->GetDesc(&descBuf);

	D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	desc.Buffer.FirstElement = 0;

	if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
	{
		// This is a Raw Buffer

		desc.Format = DXGI_FORMAT_R32_TYPELESS; // Format must be DXGI_FORMAT_R32_TYPELESS, when creating Raw Unordered Access View
		desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
		desc.Buffer.NumElements = descBuf.ByteWidth / 4;
	}
	else
	if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
	{
		// This is a Structured Buffer

		desc.Format = DXGI_FORMAT_UNKNOWN;      // Format must be must be DXGI_FORMAT_UNKNOWN, when creating a View of a Structured Buffer
		desc.Buffer.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;
	}
	else
	{
		return E_INVALIDARG;
	}

	return pDevice->CreateUnorderedAccessView(pBuffer, &desc, ppUAVOut);
}

//--------------------------------------------------------------------------------------
// Create a CPU accessible buffer and download the content of a GPU buffer into it
// This function is very useful for debugging CS programs
//-------------------------------------------------------------------------------------- 
ID3D11Buffer* MeshDeformation::CreateAndCopyToDebugBuf(ID3D11Device* pDevice, ID3D11DeviceContext* pd3dImmediateContext, ID3D11Buffer* pBuffer)
{
	ID3D11Buffer* debugbuf = NULL;

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	pBuffer->GetDesc(&desc);
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.BindFlags = 0;
	desc.MiscFlags = 0;
	if (SUCCEEDED(pDevice->CreateBuffer(&desc, NULL, &debugbuf)))
	{
#if defined(DEBUG) || defined(PROFILE)
		debugbuf->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("Debug") - 1, "Debug");
#endif

		pd3dImmediateContext->CopyResource(debugbuf, pBuffer);
	}

	return debugbuf;
}

//--------------------------------------------------------------------------------------
// Run CS
//-------------------------------------------------------------------------------------- 
void MeshDeformation::RunComputeShader(ID3D11DeviceContext* pd3dImmediateContext,
	ID3D11ComputeShader* pComputeShader,
	UINT nNumViews, ID3D11ShaderResourceView** pShaderResourceViews,
	ID3D11Buffer* pCBCS, void* pCSData, DWORD dwNumDataBytes,
	UINT nNumUnViews, ID3D11UnorderedAccessView** pUnorderedAccessView,
	UINT X, UINT Y, UINT Z)
{
	pd3dImmediateContext->CSSetShader(pComputeShader, NULL, 0);
	pd3dImmediateContext->CSSetShaderResources(0, nNumViews, pShaderResourceViews);
	pd3dImmediateContext->CSSetUnorderedAccessViews(0, nNumUnViews, pUnorderedAccessView, NULL);
	if (pCBCS)
	{
		D3D11_MAPPED_SUBRESOURCE MappedResource;
		pd3dImmediateContext->Map(pCBCS, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
		memcpy(MappedResource.pData, pCSData, dwNumDataBytes);
		pd3dImmediateContext->Unmap(pCBCS, 0);
		ID3D11Buffer* ppCB[1] = { pCBCS };
		pd3dImmediateContext->CSSetConstantBuffers(0, 1, ppCB);
	}

	pd3dImmediateContext->Dispatch(X, Y, Z);

	pd3dImmediateContext->CSSetShader(NULL, NULL, 0);

	ID3D11UnorderedAccessView* ppUAViewNULL[1] = { NULL };
	pd3dImmediateContext->CSSetUnorderedAccessViews(0, 1, ppUAViewNULL, NULL);

	ID3D11ShaderResourceView* ppSRVNULL[2] = { NULL, NULL };
	pd3dImmediateContext->CSSetShaderResources(0, 2, ppSRVNULL);

	ID3D11Buffer* ppCBNULL[1] = { NULL };
	pd3dImmediateContext->CSSetConstantBuffers(0, 1, ppCBNULL);
}

