#include "PhysicsObject.h"

PhysicsObject::PhysicsObject() : GameObject(),
//new variables
m_collisionBoxExtents(XMFLOAT3(0, 0, 0)),
m_rotationOrigin(XMFLOAT3(0, 0, 0))

{
	m_defaultContactProcessingThreshold = BT_LARGE_FLOAT;
	m_mass = 0;
}

bool PhysicsObject::CreateRigidBody(float mass, XMFLOAT3 gravity, int collisionFlags,short v1,short v2)
{
	CalcualteBoxExtents();
	if (m_collisionBoxExtents.y == 0) m_collisionBoxExtents.y = 2.0f;
	btBoxShape* groundShape = new btBoxShape(btVector3(m_collisionBoxExtents.x, m_collisionBoxExtents.y, m_collisionBoxExtents.z));
	//groundShape->initializePolyhedralFeatures();


	btTransform groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(btVector3(m_position.x, m_position.y, m_position.z));
	btQuaternion quatRot;
	quatRot.setEuler(m_yaw, m_pitch, m_roll);
	groundTransform.setRotation(quatRot);

	//setupthe object and add to the world
	m_rigidBody = localCreateRigidBody(mass, groundTransform, groundShape,v1,v2);
	m_rigidBody->setCollisionFlags(m_rigidBody->getCollisionFlags() | collisionFlags);
	m_rigidBody->setGravity(btVector3(gravity.x, gravity.y, gravity.z));
	m_rigidBody->setUserPointer(NULL);
	return true;
}

btCollisionShape* PhysicsObject::CreateCollisionShape()
{
	CalcualteBoxExtents();
	btCollisionShape* shape = new btBoxShape(btVector3(m_collisionBoxExtents.x, m_collisionBoxExtents.y, m_collisionBoxExtents.z));

	return shape;
}

btCylinderShapeX* PhysicsObject::CreateCollisionShapeCylinder()
{
	CalcualteBoxExtents();
	btCylinderShapeX* shape = new btCylinderShapeX(btVector3(m_collisionBoxExtents.x, m_collisionBoxExtents.y, m_collisionBoxExtents.z));
	return shape;
}


btTransform PhysicsObject::GetLocalTranslation()
{
	btTransform suppLocalTrans;
	suppLocalTrans.setIdentity();
	suppLocalTrans.setOrigin(btVector3(m_position.x, m_position.y, m_position.z));
	suppLocalTrans.setRotation(btQuaternion(m_yaw, m_pitch, m_roll));
	return suppLocalTrans;
}


PhysicsObject::~PhysicsObject()
{
	////dealloc btDynamicWorld
	//delete m_dynamicsWorld;
	//m_dynamicsWorld = nullptr;

	////dealloc btObjects collision shape
	//delete m_rigidBody;
	//m_rigidBody = nullptr;

}

void PhysicsObject::Update(float dt)
{
	//bulletphysics matrix needs to be casted to dxMatrix in order to keep GameObject unchanged
	//still can render with dx matrix

	//SetUp Position
	btVector3 origin;
	origin = m_rigidBody->getCenterOfMassPosition();

	//set position of object to the position for btSimulation
	m_position.x = origin.getX();
	m_position.y = origin.getY();
	m_position.z = origin.getZ();

	//SetUp Rotation
	btQuaternion Matrix;
	Matrix = m_rigidBody->getCenterOfMassTransform().getRotation();
	//calcualte the final quaternion angles
	XMMATRIX rotationI = XMMatrixRotationQuaternion(XMLoadFloat4(&XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)));
	XMMATRIX rotationMat = XMMatrixRotationQuaternion(XMLoadFloat4(&XMFLOAT4(Matrix.getX(), Matrix.getY(), Matrix.getZ(), Matrix.getW())));
	rotationMat = rotationI *  rotationMat;

	XMMATRIX originRotationOffset = XMMatrixTranslation(m_rotationOrigin.x, m_rotationOrigin.y, m_rotationOrigin.z);
	//assamble worldmat
	XMMATRIX translationMat = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	XMMATRIX scaleMat = XMMatrixScaling(m_scale, m_scale, m_scale);
	XMMATRIX worldMat = XMLoadFloat4x4(&m_worldMat);

	XMMATRIX zeroTranslation = XMMatrixTranslation(0, 0, 0);

	worldMat = originRotationOffset*
		rotationMat *
		zeroTranslation*
		scaleMat * translationMat;
	XMStoreFloat4x4(&m_worldMat, worldMat);
}


void PhysicsObject::CalcualteBoxExtents()
{

	XMFLOAT3 vertPosLowest = XMFLOAT3(0, 0, 0)
		, vertPosHighest = XMFLOAT3(0, 0, 0);
	XMFLOAT3 COM = XMFLOAT3(0, 0, 0);;
#pragma region Vposition

	if (m_model->vertexType == 1)
	{
		for (unsigned int x = 0; x < m_model->verticesPos.size(); x++)
		{
			if (m_model->verticesPos[x].position.x > vertPosHighest.x)
			{
				vertPosHighest.x = m_model->verticesPos[x].position.x;
			}

			if (m_model->verticesPos[x].position.y > vertPosHighest.y)
			{
				vertPosHighest.y = m_model->verticesPos[x].position.y;
			}

			if (m_model->verticesPos[x].position.z > vertPosHighest.z)
			{
				vertPosHighest.z = m_model->verticesPos[x].position.z;
			}


			if (m_model->verticesPos[x].position.x < vertPosLowest.x)
			{
				vertPosLowest.x = m_model->verticesPos[x].position.x;
			}

			if (m_model->verticesPos[x].position.y < vertPosLowest.y)
			{
				vertPosLowest.y = m_model->verticesPos[x].position.y;
			}

			if (m_model->verticesPos[x].position.z < vertPosLowest.z)
			{
				vertPosLowest.z = m_model->verticesPos[x].position.z;
			}
		}

		for (unsigned int x = 0; x < m_model->verticesPos.size(); x++)
		{
			COM.x += m_model->verticesPos[x].position.x;
			COM.y += m_model->verticesPos[x].position.y;
			COM.z += m_model->verticesPos[x].position.z;
		}
	}
#pragma endregion Vposition

#pragma region VpositionNormal

	if (m_model->vertexType == 2)
	{
		for (unsigned int x = 0; x < m_model->verticesPosNor.size(); x++)
		{
			if (m_model->verticesPosNor[x].position.x > vertPosHighest.x)
			{
				vertPosHighest.x = m_model->verticesPosNor[x].position.x;
			}

			if (m_model->verticesPosNor[x].position.y > vertPosHighest.y)
			{
				vertPosHighest.y = m_model->verticesPosNor[x].position.y;
			}

			if (m_model->verticesPosNor[x].position.z > vertPosHighest.z)
			{
				vertPosHighest.z = m_model->verticesPosNor[x].position.z;
			}


			if (m_model->verticesPosNor[x].position.x < vertPosLowest.x)
			{
				vertPosLowest.x = m_model->verticesPosNor[x].position.x;
			}

			if (m_model->verticesPosNor[x].position.y < vertPosLowest.y)
			{
				vertPosLowest.y = m_model->verticesPosNor[x].position.y;
			}

			if (m_model->verticesPosNor[x].position.z < vertPosLowest.z)
			{
				vertPosLowest.z = m_model->verticesPosNor[x].position.z;
			}
		}
		for (unsigned int x = 0; x < m_model->verticesPosNor.size(); x++)
		{
			COM.x += m_model->verticesPosNor[x].position.x;
			COM.y += m_model->verticesPosNor[x].position.y;
			COM.z += m_model->verticesPosNor[x].position.z;
		}
	}
#pragma endregion VpositionNormal

#pragma region VpositionTexture

	if (m_model->vertexType == 3)
	{
		for (unsigned int x = 0; x < m_model->verticesPosTex.size(); x++)
		{
			if (m_model->verticesPosTex[x].position.x > vertPosHighest.x)
			{
				vertPosHighest.x = m_model->verticesPosTex[x].position.x;
			}

			if (m_model->verticesPosTex[x].position.y > vertPosHighest.y)
			{
				vertPosHighest.y = m_model->verticesPosTex[x].position.y;
			}

			if (m_model->verticesPosTex[x].position.z > vertPosHighest.z)
			{
				vertPosHighest.z = m_model->verticesPosTex[x].position.z;
			}


			if (m_model->verticesPosTex[x].position.x < vertPosLowest.x)
			{
				vertPosLowest.x = m_model->verticesPosTex[x].position.x;
			}

			if (m_model->verticesPosTex[x].position.y < vertPosLowest.y)
			{
				vertPosLowest.y = m_model->verticesPosTex[x].position.y;
			}

			if (m_model->verticesPosTex[x].position.z < vertPosLowest.z)
			{
				vertPosLowest.z = m_model->verticesPosTex[x].position.z;
			}
		}
		for (unsigned int x = 0; x < m_model->verticesPosNor.size(); x++)
		{
			COM.x += m_model->verticesPosNor[x].position.x;
			COM.y += m_model->verticesPosNor[x].position.y;
			COM.z += m_model->verticesPosNor[x].position.z;
		}
	}
#pragma endregion VpositionTexture

#pragma region VpositionNormalTexture

	if (m_model->vertexType == 4)
	{
		for (unsigned int x = 0; x < m_model->verticesPosNorTex.size(); x++)
		{
			if (m_model->verticesPosNorTex[x].position.x > vertPosHighest.x)
			{
				vertPosHighest.x = m_model->verticesPosNorTex[x].position.x;
			}

			if (m_model->verticesPosNorTex[x].position.y > vertPosHighest.y)
			{
				vertPosHighest.y = m_model->verticesPosNorTex[x].position.y;
			}

			if (m_model->verticesPosNorTex[x].position.z > vertPosHighest.z)
			{
				vertPosHighest.z = m_model->verticesPosNorTex[x].position.z;
			}


			if (m_model->verticesPosNorTex[x].position.x < vertPosLowest.x)
			{
				vertPosLowest.x = m_model->verticesPosNorTex[x].position.x;
			}

			if (m_model->verticesPosNorTex[x].position.y < vertPosLowest.y)
			{
				vertPosLowest.y = m_model->verticesPosNorTex[x].position.y;
			}

			if (m_model->verticesPosNorTex[x].position.z < vertPosLowest.z)
			{
				vertPosLowest.z = m_model->verticesPosNorTex[x].position.z;
			}
		}
	}
#pragma endregion VpositionNormalTexture

	vertPosHighest.x *= m_scale;
	vertPosHighest.y *= m_scale;
	vertPosHighest.z *= m_scale;

	vertPosLowest.x *= m_scale;
	vertPosLowest.y *= m_scale;
	vertPosLowest.z *= m_scale;

	m_collisionBoxExtents.x = (vertPosHighest.x + vertPosLowest.x) / 2;
	m_collisionBoxExtents.y = (vertPosHighest.y + vertPosLowest.y) / 2;
	m_collisionBoxExtents.z = (vertPosHighest.z + vertPosLowest.z) / 2;

	m_collisionBoxExtents.x = (vertPosHighest.x - m_collisionBoxExtents.x);
	m_collisionBoxExtents.y = (vertPosHighest.y - m_collisionBoxExtents.y);
	m_collisionBoxExtents.z = (vertPosHighest.z - m_collisionBoxExtents.z);
}

XMFLOAT3 PhysicsObject::GetCollisionExtents()
{
	return m_collisionBoxExtents;
}

void PhysicsObject::SetRigidBody(btRigidBody * btBody)
{
	m_rigidBody = btBody;
}

void PhysicsObject::SetRotationOrigin(XMFLOAT3 rotOffset)
{
	m_rotationOrigin = rotOffset;
}

btRigidBody * PhysicsObject::GetRigidBody()
{
	return m_rigidBody;
}

btRigidBody* PhysicsObject::localCreateRigidBody(float mass, const btTransform& startTransform, btCollisionShape* shape, short v1,short v2)
{
	btAssert((!shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE));

	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0, 0, 0);
	if (isDynamic)
		shape->calculateLocalInertia(mass, localInertia);

	//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects

#define USE_MOTIONSTATE 1
#ifdef USE_MOTIONSTATE
	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);

	btRigidBody::btRigidBodyConstructionInfo cInfo(mass, myMotionState, shape, localInertia);

	btRigidBody* body = new btRigidBody(cInfo);
	body->setContactProcessingThreshold(m_defaultContactProcessingThreshold);

#else
	btRigidBody* body = new btRigidBody(mass, 0, shape, localInertia);
	body->setWorldTransform(startTransform);
#endif//

	m_dynamicsWorld->addRigidBody(body,v1,v2);

	return body;
}

void PhysicsObject::SetPosition(XMFLOAT3 position)
{
	m_position = position;
	btQuaternion quat = m_rigidBody->getCenterOfMassTransform().getRotation();
	btTransform trans = btTransform();
	btVector3 pos = btVector3(position.x, position.y, position.z);
	trans = btTransform(quat, pos);
	m_rigidBody->setCenterOfMassTransform(trans);
}

XMFLOAT4 PhysicsObject::GetQuaternion()
{
	return m_quatRotation;
}

void PhysicsObject::SetRotation(XMFLOAT3 rot)
{
	btTransform currentT;
	currentT = m_rigidBody->getWorldTransform();
	btQuaternion currentRot = btQuaternion(rot.x, rot.y, rot.z);
	currentT.setRotation(currentRot);
	m_rigidBody->setWorldTransform(currentT);
}

float PhysicsObject::GetWeight()
{
	return m_mass;
}

void PhysicsObject::SetWeight(float _mass)
{
	m_mass = _mass;
}
