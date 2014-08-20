#include "MeshDeformation.h"

MeshDeformation::MeshDeformation()
{
	m_deformType = 2;
	m_deformationType = "basic";

	m_originalMeshSum = 0;
	m_deformedMeshSum = 0;

	m_force = 1000.85f;
	m_differance = 0.0f;
	mouseX = 0;
	mouseY = 0;

	rot = XMFLOAT3(0, 0, 0);
	Y = 3;
	stop = false;


	 Cyield = 1.9f;
	 Ccreep = 0.1f;
	//how much c in order to break;
	 Cmax = 10.0f;
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

void MeshDeformation::AddObject(PhysicsObject* box, DxGraphics *dx, ResourceManager& resource, XMFLOAT3 pos)
{
	p_box = box;
	m_dynamicsWorld = resource.GetPhysicsManager()->GetWorld();
	btSphereShape* shape = new btSphereShape(0.5f);

	btTransform groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(btVector3(0, 0, 0));
	btQuaternion quatRot;
	quatRot.setEuler(0, 0, 0);
	groundTransform.setRotation(quatRot);

	//setupthe object and add to the world



	//add object to the list
	m_deformObjects.push_back(new MeshObject());
	MeshObject* GameObject = m_deformObjects[m_deformObjects.size() - 1];
	//initialize values;
	GameObject->object = box;
	GameObject->m_alpha = 0.2f;
	GameObject->m_beta = 0.7f;
	GameObject->m_deformedCOM = Vector3f(0, 0, 0);
	GameObject->m_originalCOM = Vector3f(0, 0, 0);
	GameObject->m_originalVolume = 0;
	GameObject->m_deformedVolume = 0;

	GameObject->m_Apq = Matrix3f::Identity();
	GameObject->m_Aqq = Matrix3f::Identity();

	//Copy vertecies to local control points.
	unsigned int vertexCount = GameObject->object->GetModel().verticesPosNor.size();

	for (unsigned int point = 0; point < vertexCount; point++)
	{
		GameObject->m_controlPoints.push_back(new ControlPoint());
		GameObject->m_controlPoints[point]->m_currentPos = XMFLOAT3toVector3f(GameObject->object->GetModel().verticesPosNor[point].position);
		GameObject->m_controlPoints[point]->m_goalPosition = XMFLOAT3toVector3f(GameObject->object->GetModel().verticesPosNor[point].position);
		GameObject->m_controlPoints[point]->m_originalPos = XMFLOAT3toVector3f(GameObject->object->GetModel().verticesPosNor[point].position);
		GameObject->m_controlPoints[point]->m_mass = 1.0f;
		GameObject->m_controlPoints[point]->m_relativePosDeformed = Vector3f(0, 0, 0);
		GameObject->m_controlPoints[point]->m_relativePosOriginal = Vector3f(0, 0, 0);
		GameObject->m_controlPoints[point]->velocity = Vector3f(0, 0, 0);
		GameObject->m_controlPoints[point]->m_force = Vector3f(0, 0, 0);

#define BIT(x) (1<<(x))
		enum collisiontypes {
			COL_BOX = BIT(0), //<Collide with nothing
			COL_SHPERE = BIT(1), //<Collide with ships
			COL_GROUND = BIT(2), //<Collide with walls
			COL_TEST = BIT(3)
		};

		int ShperesCollideWith = COL_BOX | COL_GROUND;
		int boxCollidesWith = COL_GROUND | COL_SHPERE | COL_BOX;
		int terrainCollidesWith = COL_SHPERE | COL_BOX | COL_TEST;
		int testBoxCollidesWith = COL_BOX | COL_GROUND;

		groundTransform.setOrigin(btVector3(GameObject->m_controlPoints[point]->m_originalPos.x(), GameObject->m_controlPoints[point]->m_originalPos.y(), GameObject->m_controlPoints[point]->m_originalPos.z()));
		btRigidBody* m_rigidBody;
		m_rigidBody = resource.GetPhysicsManager()->localCreateRigidBody(0.01f, groundTransform, shape, COL_SHPERE, ShperesCollideWith);
		m_rigidBody->setCollisionFlags(m_rigidBody->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
		m_rigidBody->setGravity(btVector3(0, -10, 0));
		m_collisionSpheres.push_back(m_rigidBody);

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
	TwAddVarRW(myBar, "plasticity", TW_TYPE_FLOAT, &Snorm, "");
	TwAddVarRW(myBar, "fromBond", TW_TYPE_FLOAT, &fromBond, "");
	//	TwAddVarRW(myBar, "FPS", TW_TYPE_INT32, &m_fps, "");

	TwAddVarRW(myBar, "Cyield", TW_TYPE_FLOAT, &Cyield, "");
	TwAddVarRW(myBar, "Ccreep", TW_TYPE_FLOAT, &Ccreep, "");
	TwAddVarRW(myBar, "Cmax", TW_TYPE_FLOAT, &Cmax, "");

#pragma endregion

	//p_Dx = dx;
	//CreateComputeShader("Deform.hlsl", "CSMain", p_Dx->GetDevice(), &g_pCS);

	//// INITIALZIE VERT
	//int NUM_ELEMENTS = m_deformObjects[m_deformObjects.size() - 1]->m_controlPoints.size();
	//g_vBufInput = new input[NUM_ELEMENTS];
	//for (int z = 0; z < NUM_ELEMENTS; z++)
	//{
	//	g_vBufInput[z].currentPosition = m_deformObjects[m_deformObjects.size() - 1]->m_controlPoints[z]->m_originalPos;
	//}

	////initial values
	//g_vBufInput2 = new values;
	//g_vBufInput2->COM = Vector3f(0, 0, 0);
	//g_vBufInput2->mass = 0;
	//g_vBufInput2->total = NUM_ELEMENTS;

	//g_vBufOutput = new output[NUM_ELEMENTS];



	////create structured buffers
	//CreateStructuredBuffer(p_Dx->GetDevice(), sizeof(input), NUM_ELEMENTS, &g_vBufInput[0], &g_pBuf0);
	//CreateStructuredBuffer(p_Dx->GetDevice(), sizeof(values), 1, &g_vBufInput2, &g_pBuf1);
	//CreateStructuredBuffer(p_Dx->GetDevice(), sizeof(output), NUM_ELEMENTS, NULL, &g_pBufResult);

	////create buffer views
	//CreateBufferSRV(p_Dx->GetDevice(), g_pBuf0, &g_pBuf0SRV);
	//CreateBufferSRV(p_Dx->GetDevice(), g_pBuf1, &g_pBuf1SRV);
	//CreateBufferUAV(p_Dx->GetDevice(), g_pBufResult, &g_pBufResultUAV);

	////run shader
	//const int buffInputCount = 2;
	//const int buffOutputCount = 1;
	//ID3D11ShaderResourceView* aRViews[buffInputCount] = { g_pBuf0SRV, g_pBuf1SRV };
	//ID3D11UnorderedAccessView* uRViews[buffOutputCount] = { g_pBufResultUAV };

	//RunComputeShader(p_Dx->GetImmediateContext(), g_pCS, buffInputCount, aRViews, NULL, NULL, 0, buffOutputCount, uRViews, 1, 1, 1);

	////Copy from gpu to local 
	//ID3D11Buffer* debugbuf = CreateAndCopyToDebugBuf(p_Dx->GetDevice(), p_Dx->GetImmediateContext(), g_pBufResult);
	//D3D11_MAPPED_SUBRESOURCE MappedResource;
	//output *p;
	//p_Dx->GetImmediateContext()->Map(debugbuf, 0, D3D11_MAP_READ, 0, &MappedResource);

	//// Set a break point here and put down the expression "p, 1024" in your watch window to see what has been written out by our CS
	//// This is also a common trick to debug CS programs.
	//p = (output*)MappedResource.pData;


}

void MeshDeformation::Update(float dt, DxGraphics *dx, DebugCamera& cam, DirectInput* input)
{
	//differance in percentage between original and deformed shape.
	//m_differance = ValueDifferance(m_originalMeshSum, m_deformedMeshSum);
	//m_deformObjects[m_deformObjects.size() - 1]->m_deformedVolume = calcVolumeDeformed();
	for each(Line* obj in m_clusterLineList)
	{
		obj->Update(dt);
	}
#pragma region InputChange

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

	if (input->GetKeyboardState(DIK_5) && input->GetKeyboardPrevState(DIK_5))
	{
		m_deformType = 4;
	}

	if ((input->GetKeyboardState(DIK_F)) &&
		(!input->GetKeyboardPrevState(DIK_F)))
	{
		if (stop) stop = false;
		else stop = true;
	}

	if ((input->GetKeyboardState(DIK_R)) &&
		(!input->GetKeyboardPrevState(DIK_R)))
	{
		for (unsigned int point = 0; point < m_deformObjects[m_deformObjects.size() - 1]->m_controlPoints.size(); point++)
		{
			m_deformObjects[m_deformObjects.size() - 1]->object->GetModel().verticesPosNor[point].position = Vector3ftoXMFLOAT3(m_deformObjects[m_deformObjects.size() - 1]->m_controlPoints[point]->m_originalPos);
			m_deformObjects[m_deformObjects.size() - 1]->m_controlPoints[point]->m_currentPos = m_deformObjects[m_deformObjects.size() - 1]->m_controlPoints[point]->m_originalPos;
			btVector3 velocity = btVector3(0, 0, 0);
			m_collisionSpheres[point]->setLinearVelocity(velocity);


			btTransform colTrans;
			colTrans.setOrigin(btVector3(m_deformObjects[m_deformObjects.size() - 1]->m_controlPoints[point]->m_currentPos.x(), m_deformObjects[m_deformObjects.size() - 1]->m_controlPoints[point]->m_currentPos.y(), m_deformObjects[m_deformObjects.size() - 1]->m_controlPoints[point]->m_currentPos.z()));
			m_collisionSpheres[point]->setCenterOfMassTransform(colTrans);
			m_particles[point]->SetPosition(Vector3ftoXMFLOAT3(m_deformObjects[m_deformObjects.size() - 1]->m_controlPoints[point]->m_currentPos) + m_deformObjects[m_deformObjects.size() - 1]->object->GetPosition());
			m_particles[point]->Update(dt);
		}
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
				  for (unsigned int point = 0; point < obj->m_controlPoints.size(); point++)
				  {
					  btTransform colTrans;
					  colTrans = m_collisionSpheres[point]->getWorldTransform();
					  btVector3 newPos = colTrans.getOrigin();

					  obj->m_controlPoints[point]->m_currentPos = Vector3f(newPos.getX(), newPos.getY(), newPos.getZ());

				  }

				  //Calcualte Deformed shape 
				  CalculateDeformedCom(obj);
				  //Calcualte relative psition of defomred points.
				  Calculate_p(obj);
				  CalcualteApqMatrix(obj);

				  //Calcualte RotationMatrix
				  Matrix3f RotMat = XMMATRIXtoMatrix3f(ComputeOptimumRotation(Matrix3ftoXMMATRIX(obj->m_Apq)));

				  for (unsigned int point = 0; point < obj->m_controlPoints.size(); point++)
				  {
					  //Rotate original position of the control point by extracted Rotation matrix.
					  Vector3f goalPosition = rotateVect(RotMat, obj->m_controlPoints[point]->m_relativePosOriginal);

					  //Add the worldspace (CenterOfMass) Position to local vertex position
					  goalPosition = goalPosition + obj->m_originalCOM;

					  //Assign New goalposition for a contol point
					  obj->m_controlPoints[point]->m_goalPosition = goalPosition;

					  //Update point //veocity//position..
					  PointUpdate(obj->m_controlPoints[point], dt);

					  //update model vertecies
					  obj->object->GetModel().verticesPosNor[point].position = Vector3ftoXMFLOAT3(obj->m_controlPoints[point]->m_currentPos);


					  ///COLLISION UPDATE
					  btVector3 velocity;
					  Vector3f oVel = obj->m_controlPoints[point]->velocity;
					  velocity = btVector3(oVel.x(), oVel.y(), oVel.z());
					  m_collisionSpheres[point]->setLinearVelocity(velocity);

					  btTransform colTrans;
					  colTrans.setOrigin(btVector3(obj->m_controlPoints[point]->m_currentPos.x(), obj->m_controlPoints[point]->m_currentPos.y(), obj->m_controlPoints[point]->m_currentPos.z()));
					  m_collisionSpheres[point]->setCenterOfMassTransform(colTrans);
					  m_particles[point]->SetPosition(Vector3ftoXMFLOAT3(obj->m_controlPoints[point]->m_currentPos) + obj->object->GetPosition());
					  m_particles[point]->Update(dt);
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
				  for (unsigned int point = 0; point < obj->m_controlPoints.size(); point++)
				  {
					  btTransform colTrans;
					  colTrans = m_collisionSpheres[point]->getWorldTransform();
					  btVector3 newPos = colTrans.getOrigin();

					  obj->m_controlPoints[point]->m_currentPos = Vector3f(newPos.getX(), newPos.getY(), newPos.getZ());

				  }

				  CalcualteDeformedMeshSum(obj);

				  //calcualte center of mass for a deformed shape
				  CalculateDeformedCom(obj);
				  //calcualte control point position relative to the center of the deformed shape
				  Calculate_p(obj);
				  //transformation matrix
				  CalcualteApqMatrix(obj);

				  //linear transformation matrix
				  Matrix3f A = obj->m_Apq * obj->m_Aqq;
				  //Extract Optimal Rotation from the transformation matrix of the deformed shape.
				  Matrix3f RotMat = XMMATRIXtoMatrix3f(ComputeOptimumRotation(Matrix3ftoXMMATRIX(obj->m_Apq)));
				  //Linear TransformationMatrix
				  Matrix3f LinearTransform = volumeNormalize(A);
				  Matrix3f transform;

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
					  //deformed relative position.
					  Vector3f q = obj->m_controlPoints[point]->m_relativePosOriginal;
					  //rotate the shape by the extracted rotation matrix.
					  Vector3f goalPosition = rotateVect(transform, q);
					  //Set the shape to the original Position.
					  goalPosition = goalPosition + obj->m_originalCOM;
					  obj->m_controlPoints[point]->m_goalPosition = goalPosition;
					  ////set the new control point position
					  PointUpdate(obj->m_controlPoints[point], dt);
					  //update model vertecies
					  obj->object->GetModel().verticesPosNor[point].position = Vector3ftoXMFLOAT3(obj->m_controlPoints[point]->m_currentPos);

					  ///COLLISION SPHERE UPDATE
					  btVector3 velocity;
					  Vector3f oVel = obj->m_controlPoints[point]->velocity;
					  velocity = btVector3(oVel.x(), oVel.y(), oVel.z());
					  m_collisionSpheres[point]->setLinearVelocity(velocity);


					  btTransform colTrans;
					  colTrans.setOrigin(btVector3(obj->m_controlPoints[point]->m_currentPos.x(), obj->m_controlPoints[point]->m_currentPos.y(), obj->m_controlPoints[point]->m_currentPos.z()));
					  m_collisionSpheres[point]->setCenterOfMassTransform(colTrans);
					  m_particles[point]->SetPosition(Vector3ftoXMFLOAT3(obj->m_controlPoints[point]->m_currentPos) + obj->object->GetPosition());
					  m_particles[point]->Update(dt);
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
				  if (!stop)
				  {

					  XMFLOAT3 worldPos = p_box->GetPosition();
					  for (unsigned int point = 0; point < obj->m_controlPoints.size(); point++)
					  {
						  btTransform colTrans;
						  colTrans = m_collisionSpheres[point]->getWorldTransform();
						  btVector3 newPos = colTrans.getOrigin();

						  obj->m_controlPoints[point]->m_currentPos = Vector3f(newPos.getX(), newPos.getY(), newPos.getZ());
					  }

					  //Center of mass of the deformed shape
					  CalculateDeformedCom(obj);
					  //Relative position of each deformed aprticle to center of mass of the deformed shape.
					  Calculate_p(obj);

					  Calculate_q_tilde(obj);
					  //TANKS FRAMES
					  CalcualteAqqMatrix_Tilde(*obj);
					  CalcualteApqMatrix_Tilde(*obj);


					  //Matrix [AQM] A-optimal Linear transofmration, Q - coefficents of pure quadratic terms, M - coeficents of Mixed terms.
					  //Assamble AQM Matrix. 
					  obj->AQM = obj->m_Apq_tilde * obj->m_Aqq_tilde;

					  //extract Linear Transofrmation matrix from [AQM] R(3x3). (to calcualte optimal rotation)
					  for (unsigned int row = 0; row < 3; row++)
					  {
						  for (unsigned int col = 0; col < 3; col++)
						  {
							  obj->A(row, col) = obj->AQM(row, col);
						  }
					  }
					  //ensure volume is preserved;
					  obj->A = volumeNormalize(obj->A);
					  //assamble back into AQM
					  for (unsigned int row = 0; row < 3; row++)
					  {
						  for (unsigned int col = 0; col < 3; col++)
						  {
							  obj->AQM(row, col) = obj->A(row, col);
						  }
					  }

					  // calcualte optiaml rotation matrix 
					  obj->Rotation = XMMATRIXtoMatrix3f(ComputeOptimumRotation(Matrix3ftoXMMATRIX(obj->A)));


					  MatrixXf Rt(3, 9);
					  for (unsigned int row = 0; row < 3; row++)
					  {
						  for (unsigned int col = 0; col < 3; col++)
						  {
							  Rt(row, col) = obj->Rotation(row, col);
						  }
					  }
					  //populate rest with 0
					  for (unsigned int row = 0; row < 3; row++)
					  {
						  for (unsigned int col = 3; col < 9; col++)
						  {
							  Rt(row, col) = 0;
						  }
					  }


					  MatrixXf Ttilde(3, 9);
					  Ttilde = m_deformObjects[m_deformObjects.size() - 1]->m_beta* obj->AQM + (1 - m_deformObjects[m_deformObjects.size() - 1]->m_beta) * Rt;


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

						  MatrixXf g2 = Ttilde * obj->m_controlPoints[point]->Qmat;
						  Vector3f goalPosition = Vector3f((float)g2(0, 0), (float)g2(1, 0), (float)g2(2, 0));
						  //Offset to deformed position.
						  goalPosition = goalPosition + obj->m_originalCOM;

						  //Assign New goalposition 
						  obj->m_controlPoints[point]->m_goalPosition = goalPosition;

						  //Update point
						  PointUpdate(obj->m_controlPoints[point], dt);
						  //update model vertecies
						  obj->object->GetModel().verticesPosNor[point].position = Vector3ftoXMFLOAT3(obj->m_controlPoints[point]->m_currentPos);

						  btVector3 velocity;
						  Vector3f oVel = obj->m_controlPoints[point]->velocity;
						  velocity = btVector3(oVel.x(), oVel.y(), oVel.z());
						  m_collisionSpheres[point]->setLinearVelocity(velocity);


						  btTransform colTrans;
						  colTrans.setOrigin(btVector3(obj->m_controlPoints[point]->m_currentPos.x(), obj->m_controlPoints[point]->m_currentPos.y(), obj->m_controlPoints[point]->m_currentPos.z()));
						  m_collisionSpheres[point]->setCenterOfMassTransform(colTrans);
						  m_particles[point]->SetPosition(Vector3ftoXMFLOAT3(obj->m_controlPoints[point]->m_currentPos) + obj->object->GetPosition());
						  m_particles[point]->Update(dt);
					  }
				  }

			  }
			  break;
	}

#pragma endregion

#pragma region ClusteringTry

	case 3:
	{
			  m_deformationType = "CLUSTERNIG - LINEAR";
			  //LINEAR DEFORMATION

			  for each (MeshObject* obj in m_deformObjects)
			  {
				  for (unsigned int point = 0; point < obj->m_controlPoints.size(); point++)
				  {
					  btTransform colTrans;
					  colTrans = m_collisionSpheres[point]->getWorldTransform();
					  btVector3 newPos = colTrans.getOrigin();

					  obj->m_controlPoints[point]->m_currentPos = Vector3f(newPos.getX(), newPos.getY(), newPos.getZ());

				  }

				  CalcualteDeformedMeshSum(obj);

				  //calcualte center of mass for a deformed shape
				  CalculateDeformedCom(obj);
				  //calcualte control point position relative to the center of the deformed shape
				  Calculate_p(obj);
				  //transformation matrix
				  CalcualteApqMatrix(obj);

				  //linear transformation matrix
				  Matrix3f A = obj->m_Apq * obj->m_Aqq;
				  //Extract Optimal Rotation from the transformation matrix of the deformed shape.
				  Matrix3f RotMat = XMMATRIXtoMatrix3f(ComputeOptimumRotation(Matrix3ftoXMMATRIX(obj->m_Apq)));
				  //Linear TransformationMatrix
				  Matrix3f LinearTransform = volumeNormalize(A);
				  Matrix3f transform;

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
					  //deformed relative position.
					  Vector3f q = obj->m_controlPoints[point]->m_relativePosOriginal;
					  //rotate the shape by the extracted rotation matrix.
					  Vector3f goalPosition = rotateVect(transform, q);
					  //Set the shape to the original Position.
					  goalPosition = goalPosition + obj->m_originalCOM;
					  obj->m_controlPoints[point]->m_goalPosition = goalPosition;
					  ////set the new control point position
					  PointUpdate(obj->m_controlPoints[point], dt);
					  //update model vertecies
					  obj->object->GetModel().verticesPosNor[point].position = Vector3ftoXMFLOAT3(obj->m_controlPoints[point]->m_currentPos);

					  ///COLLISION SPHERE UPDATE
					  btVector3 velocity;
					  Vector3f oVel = obj->m_controlPoints[point]->velocity;
					  velocity = btVector3(oVel.x(), oVel.y(), oVel.z());
					  m_collisionSpheres[point]->setLinearVelocity(velocity);


					  btTransform colTrans;
					  colTrans.setOrigin(btVector3(obj->m_controlPoints[point]->m_currentPos.x(), obj->m_controlPoints[point]->m_currentPos.y(), obj->m_controlPoints[point]->m_currentPos.z()));
					  m_collisionSpheres[point]->setCenterOfMassTransform(colTrans);
					  m_particles[point]->SetPosition(Vector3ftoXMFLOAT3(obj->m_controlPoints[point]->m_currentPos) + obj->object->GetPosition());
					  m_particles[point]->Update(dt);
				  }
			  }
			  break;
	}
#pragma endregion

#pragma region Plasticity

	case 4:
	{
			  m_deformationType = "PLASTICITY - LINEAR";
			  //LINEAR DEFORMATION

			  for each (MeshObject* obj in m_deformObjects)
			  {
				  for (unsigned int point = 0; point < obj->m_controlPoints.size(); point++)
				  {
					  btTransform colTrans;
					  colTrans = m_collisionSpheres[point]->getWorldTransform();
					  btVector3 newPos = colTrans.getOrigin();

					  obj->m_controlPoints[point]->m_currentPos = Vector3f(newPos.getX(), newPos.getY(), newPos.getZ());

				  }

				  CalcualteDeformedMeshSum(obj);

				  //calcualte center of mass for a deformed shape
				  CalculateDeformedCom(obj);
				  //calcualte control point position relative to the center of the deformed shape
				  Calculate_p(obj);
				  //transformation matrix
				  CalcualteApqMatrix(obj);

				  //linear transformation matrix
				  Matrix3f A = obj->m_Apq * obj->m_Aqq;
				  //Extract Optimal Rotation from the transformation matrix of the deformed shape.
				  Matrix3f RotMat = XMMATRIXtoMatrix3f(ComputeOptimumRotation(Matrix3ftoXMMATRIX(obj->m_Apq)));
				  //Linear TransformationMatrix
				  Matrix3f LinearTransform = volumeNormalize(A);
				  Matrix3f transform;

				  for (unsigned int x = 0; x < 3; x++)
				  {
					  for (unsigned int y = 0; y < 3; y++)
					  {
						  LinearTransform(x, y) *= m_deformObjects[m_deformObjects.size() - 1]->m_beta;
						  RotMat(x, y) *= (1 - m_deformObjects[m_deformObjects.size() - 1]->m_beta);
						  transform(x, y) = LinearTransform(x, y) + RotMat(x, y);
					  }
				  }

				  Matrix3f Identitiy = Matrix3f::Identity();
				  Matrix3f Splasticity = Identitiy;
				  //Controll plasticity.


				  Matrix3f Smat = RotMat.transpose() * A;

				  ///////////////////////////////////////////////////////
				  //IF EXCEEDS CIELD FIRST
				  Snorm = (Smat - Identitiy).lpNorm<2>();
				  if (Snorm > Cyield)
				  {
					  Splasticity = (Identitiy + dt * Ccreep* (Smat * Identitiy)) * Splasticity;
				  }
				  //bond by testing
				  fromBond = (Splasticity - Identitiy).lpNorm<2>();
				  if (fromBond > Cmax)
				  {
					  Splasticity = Identitiy + Cmax * (Splasticity - Identitiy) / (Splasticity - Identitiy).lpNorm<2>();
				  }
				  Splasticity = volumeNormalize(Splasticity);
				  ///////////////////////////////////////////////////////
				  ////UPDATE AQQ////////////////////
				  //RELATIVE UPDATE///
				  //Update relative point positions of Original shape
				  for (unsigned int point = 0; point < obj->m_controlPoints.size(); point++)
				  {
					  obj->m_controlPoints[point]->m_relativePosOriginal = Splasticity * (obj->m_controlPoints[point]->m_originalPos - obj->m_deformedCOM);
				  }
				  //////AQQ UPDATE////////////////////
				  Matrix3f AqqInverse = Matrix3f::Identity();
				  //mass of a point
				  float* mass;
				  //	position of a point relative to center of massass
				  Vector3f q;
				  //determinant need a parameter
				  XMVECTOR det;
				  for (unsigned int point = 0; point < obj->m_controlPoints.size(); point++)
				  {
					  mass = &obj->m_controlPoints[point]->m_mass;
					  q = obj->m_controlPoints[point]->m_relativePosOriginal;
					  AqqInverse(0, 0) += *mass * q.x() * q.x();
					  AqqInverse(0, 1) += *mass * q.y() * q.x();
					  AqqInverse(0, 2) += *mass * q.z() * q.x();

					  AqqInverse(1, 0) += *mass * q.x() * q.y();
					  AqqInverse(1, 1) += *mass * q.y() * q.y();
					  AqqInverse(1, 2) += *mass * q.z() * q.y();

					  AqqInverse(2, 0) += *mass * q.x() * q.z();
					  AqqInverse(2, 1) += *mass * q.y() * q.z();
					  AqqInverse(2, 2) += *mass * q.z() * q.z();
				  }
				  obj->m_Aqq = AqqInverse.inverse();
				  //////////////////////////////////////////////////////////

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
					  Vector3f q = obj->m_controlPoints[point]->m_originalPos;
					  //rotate the shape by the extracted rotation matrix.
					  Vector3f goalPosition = rotateVect(transform, q);
					  //Set the shape to the original Position.
					  goalPosition = goalPosition + obj->m_originalCOM;
					  obj->m_controlPoints[point]->m_goalPosition = goalPosition;
					  ////set the new control point position
					  PointUpdate(obj->m_controlPoints[point], dt);
					  //update model vertecies
					  obj->object->GetModel().verticesPosNor[point].position = Vector3ftoXMFLOAT3(obj->m_controlPoints[point]->m_currentPos);


					  m_particles[point]->SetPosition(Vector3ftoXMFLOAT3(obj->m_controlPoints[point]->m_currentPos) + obj->object->GetPosition());
					  //Update point
					  m_particles[point]->Update(dt);
					  //set visual point positions
				


				  }
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
		//	p->Render(dx, mCam);
	}

	for (unsigned int x = 0; x < m_clusterLineList.size(); x++)
	{
		//m_clusterLineList[x]->Render(dx, &mCam);
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
		object->m_controlPoints[point]->m_relativePosOriginal = object->m_controlPoints[point]->m_originalPos - object->m_originalCOM;
	}
}

void MeshDeformation::CalcualteApqMatrix(MeshObject* object)
{
	//Calcualte Apq Matrix ( rotation information )
	Matrix3f Apq = Matrix3f::Identity();
	// store mass of each point
	float* mass;

	for (unsigned int point = 0; point < object->m_controlPoints.size(); point++)
	{
		mass = &object->m_controlPoints[point]->m_mass;
		Vector3f q = object->m_controlPoints[point]->m_relativePosOriginal;
		Vector3f p = object->m_controlPoints[point]->m_relativePosDeformed;
		Apq(0, 0) += *mass * p.x() * q.x();
		Apq(0, 1) += *mass * p.y() * q.x();
		Apq(0, 2) += *mass * p.z() * q.x();

		Apq(1, 0) += *mass * p.x() * q.y();
		Apq(1, 1) += *mass * p.y() * q.y();
		Apq(1, 2) += *mass * p.z() * q.y();

		Apq(2, 0) += *mass * p.x() * q.z();
		Apq(2, 1) += *mass * p.y() * q.z();
		Apq(2, 2) += *mass * p.z() * q.z();
	}
	object->m_Apq = Apq;
}
void MeshDeformation::CalcualteAqqMatrix(MeshObject* object)
{
	Matrix3f AqqInverse = Matrix3f::Identity();
	//mass of a point
	float* mass;
	//	position of a point relative to center of massass
	Vector3f q;
	for (unsigned int point = 0; point < object->m_controlPoints.size(); point++)
	{
		mass = &object->m_controlPoints[point]->m_mass;
		q = object->m_controlPoints[point]->m_relativePosOriginal;
		AqqInverse(0, 0) += *mass * q.x() * q.x();
		AqqInverse(0, 1) += *mass * q.y() * q.x();
		AqqInverse(0, 2) += *mass * q.z() * q.x();

		AqqInverse(1, 0) += *mass * q.x() * q.y();
		AqqInverse(1, 1) += *mass * q.y() * q.y();
		AqqInverse(1, 2) += *mass * q.z() * q.y();

		AqqInverse(2, 0) += *mass * q.x() * q.z();
		AqqInverse(2, 1) += *mass * q.y() * q.z();
		AqqInverse(2, 2) += *mass * q.z() * q.z();
	}
	object->m_Aqq = AqqInverse.inverse();
}

//Calcualte CenterOfMass of Original and Deformed shapes.
void MeshDeformation::CalcualteOriginalCom(MeshObject* object)
{

	//Calcualte center of mass for original shape
	Vector3f positionSum = Vector3f(0, 0, 0);
	float massSum = 0;

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
	Vector3f positionSum = Vector3f(0, 0, 0);
	float massSum = 0;

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
	MatrixXf Q_mat(9, 1);
	//store not to call;
	Vector3f relativePos;
	//Update relative point positions of Original shape
	for (unsigned int point = 0; point < object->m_controlPoints.size(); point++)
	{
		relativePos = object->m_controlPoints[point]->m_relativePosOriginal;

		Q_mat << 
		relativePos.x(),
		relativePos.y(),
		relativePos.z(),

		relativePos.x() * relativePos.x(),
		relativePos.y() * relativePos.y(),
		relativePos.z() * relativePos.z(),

		relativePos.x() * relativePos.y(),
		relativePos.y() * relativePos.z(),
		relativePos.z() * relativePos.x();

		object->m_controlPoints[point]->Qmat = Q_mat;
	}
}

void MeshDeformation::CalcualteAqqMatrix_Tilde(MeshObject& object)
{
	MatrixXf AqqInverse(9, 9);
	AqqInverse = MatrixXf::Identity(9, 9);
	VectorXf qPosition;

	MatrixXf qT(9, 1);
	qT = MatrixXf::Identity(9, 1);

	//mass of a prticular controll point
	float mass;

	//loop through each controll point in order to calculate the Matrix
	for (unsigned int point = 0; point < object.m_controlPoints.size(); point++)
	{
		mass = object.m_controlPoints[point]->m_mass;
		qPosition = object.m_controlPoints[point]->m_relativePosOriginal;

		qT << qPosition.x(),
			qPosition.y(),
			qPosition.z(),
			qPosition.x() * qPosition.x(),
			qPosition.y() * qPosition.y(),
			qPosition.z() * qPosition.z(),
			qPosition.x() * qPosition.y(),
			qPosition.y() * qPosition.z(),
			qPosition.z() * qPosition.x();

		AqqInverse.noalias() += mass * qT * qT.transpose();
	}
	object.m_Aqq_tilde = AqqInverse.inverse();
}
void MeshDeformation::CalcualteApqMatrix_Tilde(MeshObject& object)
{
	MatrixXf Apq(3, 9);
	Apq = MatrixXf::Identity(3, 9);
	//ROw COlls
	MatrixXf pp(3, 1);
	pp = MatrixXf::Identity(3, 1);

	Vector3f q;
	MatrixXf qT(1, 9);
	qT = MatrixXf::Identity(1, 9);

	float m;
	Vector3f p;
	for (unsigned int point = 0; point < object.m_controlPoints.size(); point++)
	{
		m = object.m_controlPoints[point]->m_mass;
		p = object.m_controlPoints[point]->m_relativePosDeformed;

		pp << p.x(), p.y(), p.z();

		q = object.m_controlPoints[point]->m_relativePosOriginal;

		qT << q.x(),
			q.y(),
			q.z(),
			q.x()*q.x(),
			q.y()*q.y(),
			q.z()*q.z(),
			q.x() * q.y(),
			q.y() * q.z(),
			q.z()*q.x();

		Apq += m * pp *qT;
	}

	object.m_Apq_tilde = Apq;
}

void MeshDeformation::PointUpdate(ControlPoint* point, float dt)
{
	Vector3f velocity = point->velocity;
	velocity = velocity + ((((point->m_goalPosition - point->m_currentPos) * m_deformObjects[m_deformObjects.size() - 1]->m_alpha) / dt)) + (point->m_goalPosition - point->m_force*dt / point->m_mass);

	Vector3f pos = point->m_currentPos;
	Vector3f t = velocity * dt;
	Vector3f newPos = pos + t;
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
	Vector3f positionSum = Vector3f(0, 0, 0);
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
		Vector3f q = GameObject->m_controlPoints[_cluster.m_vertecies[x]]->m_relativePosOriginal;
		AqqInverse(0, 0) += m * q.x() * q.x();
		AqqInverse(0, 1) += m * q.y() * q.x();
		AqqInverse(0, 2) += m * q.z() * q.x();

		AqqInverse(1, 0) += m * q.x() * q.y();
		AqqInverse(1, 1) += m * q.y() * q.y();
		AqqInverse(1, 2) += m * q.z() * q.y();

		AqqInverse(2, 0) += m * q.x() * q.z();
		AqqInverse(2, 1) += m * q.y() * q.z();
		AqqInverse(2, 2) += m * q.z() * q.z();
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
	Vector3f positionSum = Vector3f(0, 0, 0);
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
		Vector3f q = GameObject->m_controlPoints[_cluster.m_vertecies[x]]->m_relativePosOriginal;
		Vector3f p = GameObject->m_controlPoints[_cluster.m_vertecies[x]]->m_relativePosDeformed;
		Apq(0, 0) += m * p.x() * q.x();
		Apq(0, 1) += m * p.y() * q.x();
		Apq(0, 2) += m * p.z() * q.x();

		Apq(1, 0) += m * p.x() * q.y();
		Apq(1, 1) += m * p.y() * q.y();
		Apq(1, 2) += m * p.z() * q.y();

		Apq(2, 0) += m * p.x() * q.z();
		Apq(2, 1) += m * p.y() * q.z();
		Apq(2, 2) += m * p.z() * q.z();
	}
	XMStoreFloat4x4(&_cluster.m_Apq, Apq);
}

void MeshDeformation::ClusterCalutate(DxGraphics* dx, ResourceManager & resource)
{
	//vector<int> ClusterTop, ClusterBottom;

	////Divide vertex buddef by clustnum,separate 
	//XMFLOAT3 numOfClusters = XMFLOAT3(1, 2, 1);
	////create 2 clusters
	//m_clusterList.push_back(Cluster());
	//m_clusterList.push_back(Cluster());

	//for (unsigned int x = 0; x < m_deformObjects[m_deformObjects.size() - 1]->object->GetModel().verticesPosNor.size(); x++)
	//{

	//	ClusterBottom.push_back(x);



	//	ClusterTop.push_back(x);

	//}

	////ASSIGN THE CLUSTERS
	//m_clusterList[1].m_vertecies = ClusterTop;
	//m_clusterList[0].m_vertecies = ClusterBottom;

	//for (unsigned int x = 0; x < m_clusterList.size(); x++)
	//{

	//	//calcualte original center of mass of the non deformed shape
	//	CalcualteOriginalComCluster(m_clusterList[x]);
	//	//calcualte relative aprticle positions
	//	Calculate_qCluster(m_clusterList[x]);
	//	//calcualte scaling matrix
	//	CalcualteAqqMatrixCluster(m_clusterList[x]);
	//}

	Vector3f min = Vector3f(0, 0, 0), max = Vector3f(0, 0, 0);
	for (unsigned int xx = 0; xx < m_deformObjects.size(); xx++)
	{

		for (unsigned int y = 0; y < m_deformObjects[m_deformObjects.size() - 1]->m_controlPoints.size(); y++)
		{
			Vector3f position = m_deformObjects[m_deformObjects.size() - 1]->m_controlPoints[y]->m_originalPos;
			//m_deformObjects[m_deformObjects.size() - 1]->object->GetModel().verticesPosNor[y].colour = XMFLOAT4(1, 0, 0, 1);
			if (position.x() < min.x())
			{
				min.x() = position.x();
			}
			if (position.y() < min.y())
			{
				min.y() = position.y();
			}
			if (position.z() < min.z())
			{
				min.z() = position.z();
			}

			if (position.x() > max.x())
			{
				max.x() = position.x();
			}
			if (position.y() > max.y())
			{
				max.y() = position.y();
			}
			if (position.z() > max.z())
			{
				max.z() = position.z();
			}
		}


		Vector3f minLocal;
		minLocal.x() = abs(min.x());
		minLocal.y() = abs(min.y());
		minLocal.z() = abs(min.z());
		//size of one cluster
		Vector3f clusterSize = (minLocal + max);
		Vector3f clusterAmmount = Vector3f(2,2,1);
		clusterSize.x() /= clusterAmmount.x();
		clusterSize.y() /= clusterAmmount.y();
		clusterSize.z() /= clusterAmmount.z();
	
		XMFLOAT3 maxBox, minBox;
		//for (int clusterY = 0; clusterY < clusterAmmount.y(); clusterY++)
		//{
		//		minBox.y = (min.y() + clusterY * clusterSize.y());
		//		maxBox.y =  (min.y() + (clusterY + 1) * clusterSize.y());
		//}

		//for (int clusterX = 0; clusterX < clusterAmmount.x(); clusterX++)
		//{
		//		minBox.x =  (min.x() + clusterX * clusterSize.x());
		//		maxBox.x =  (min.x() + (clusterX + 1) * clusterSize.x());
		//}


		//for (int clusterZ = 0; clusterZ < clusterAmmount.z(); clusterZ++)
		//{
		//		minBox.z =  (min.z() + clusterZ * clusterSize.z());
		//		maxBox.z =  (min.z() + (clusterZ + 1) * clusterSize.z());
		//}
		float aa = clusterAmmount.x();
		if (aa == 1)
		{ 
			clusterAmmount.x() = 0;
		}

		float bb = clusterAmmount.y();
		if (bb == 1) { clusterAmmount.y() = 0; }

		float cc = clusterAmmount.z();
		if (cc == 1){ clusterAmmount.z() = 0; }
		
		float total = clusterAmmount.x() + clusterAmmount.y() + clusterAmmount.z();
		float current = 0;
		Vector3f currentCluster = Vector3f(0, 0, 0);
		while (current < total)
		{
			current++;
			minBox.x = (min.x() + currentCluster.x() * clusterSize.x());
			maxBox.x = (min.x() + (currentCluster.x() + 1) * clusterSize.x());

			
			minBox.y = (min.y() + currentCluster.y() * clusterSize.y());
			maxBox.y = (min.y() + (currentCluster.y() + 1) * clusterSize.y());

			
			minBox.z = (min.z() + currentCluster.z() * clusterSize.z());
			maxBox.z = (min.z() + (currentCluster.z() + 1) * clusterSize.z());

			m_clusterLineList.push_back(new Line());
			m_clusterLineList[m_clusterLineList.size() - 1]->LoadContent(dx, resource);
			m_clusterLineList[m_clusterLineList.size() - 1]->SetSize(minBox, maxBox);
			if (currentCluster.x() < clusterAmmount.x()){ currentCluster.x()++;  }
			if (currentCluster.y() < clusterAmmount.y()) { currentCluster.y()++;  }
			if (currentCluster.z() < clusterAmmount.z()) { currentCluster.z()++;  }
		}


	}
}



//MESH SUM CALACULATION
void MeshDeformation::CalcualteOriginalMeshSum(MeshObject* object)
{
	m_originalMeshSum = 0;
	for (unsigned int x = 0; x < object->m_controlPoints.size(); x++)
	{
		m_originalMeshSum = (object->m_controlPoints[x]->m_originalPos.x() + object->m_controlPoints[x]->m_originalPos.y() + object->m_controlPoints[x]->m_originalPos.z());
	}
	m_originalMeshSum = m_originalMeshSum / object->m_controlPoints.size();
}

void MeshDeformation::CalcualteDeformedMeshSum(MeshObject* object)
{
	m_deformedMeshSum = 0;
	for (unsigned int x = 0; x < object->m_controlPoints.size(); x++)
	{

		m_deformedMeshSum = (object->m_controlPoints[x]->m_goalPosition.x() + object->m_controlPoints[x]->m_goalPosition.y() + object->m_controlPoints[x]->m_goalPosition.z());
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
		Vector3f p1 = m_deformObjects[m_deformObjects.size() - 1]->m_controlPoints[point + 0]->m_originalPos;
		Vector3f p2 = m_deformObjects[m_deformObjects.size() - 1]->m_controlPoints[point + 1]->m_originalPos;
		Vector3f p3 = m_deformObjects[m_deformObjects.size() - 1]->m_controlPoints[point + 2]->m_originalPos;

		float v321 = p3.x() * p2.y() * p1.z();
		float v231 = p2.x() * p3.y() * p1.z();
		float v312 = p3.x() * p1.y() * p2.z();
		float v132 = p1.x() * p3.y() * p2.z();
		float v213 = p2.x() * p1.y() * p3.z();
		float v123 = p1.x() * p2.y() * p3.z();

		volume += (1.0f / 6.0f) * (-v321 + v231 + v312 - v132 - v213 + v123);
	}

	return abs(volume);
}

float MeshDeformation::calcVolumeDeformed()
{
	float volume = 0;

	for (int point = 0; point < m_deformObjects[m_deformObjects.size() - 1]->m_controlPoints.size() / 3; point += 3)
	{
		Vector3f p1 = m_deformObjects[m_deformObjects.size() - 1]->m_controlPoints[point + 0]->m_goalPosition;
		Vector3f p2 = m_deformObjects[m_deformObjects.size() - 1]->m_controlPoints[point + 1]->m_goalPosition;
		Vector3f p3 = m_deformObjects[m_deformObjects.size() - 1]->m_controlPoints[point + 2]->m_goalPosition;

		float v321 = p3.x() * p2.y() * p1.z();
		float v231 = p2.x() * p3.y() * p1.z();
		float v312 = p3.x() * p1.y() * p2.z();
		float v132 = p1.x() * p3.y() * p2.z();
		float v213 = p2.x() * p1.y() * p3.z();
		float v123 = p1.x() * p2.y() * p3.z();

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

