#ifndef PHYSICSOBJECT_H
#define PHYSICSOBJECT_H

#include "GameObject.h"
#include "VertexTypes.h"
#include <vector>

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
//#include "btBulletDynamicsCommon.h"
using namespace std;


class PhysicsObject : public GameObject
{

public:
	PhysicsObject();
	virtual ~PhysicsObject();

	virtual void Update(float dt);

	//used to add a initialize and add a collidibale object to the physics simualtion.
	btRigidBody* localCreateRigidBody(float mass, const btTransform& startTransform, btCollisionShape* shape);
	//Calcualte collision box extens used for bulletphysics
	void CalcualteBoxExtents();
	//Getter fo the collision extents
	XMFLOAT3 GetCollisionExtents();

	//each object has a default collision shape.
	void SetRigidBody(btRigidBody * btBody);
	btRigidBody * GetRigidBody();

	//Update specifically uses Quaternion Rotations.
	//void SetQuaternion(XMFLOAT4 quat);
	XMFLOAT4 GetQuaternion();

	//each phsics object has a collision shape
	btRigidBody* m_rigidBody;


	bool CreateRigidBody(float mass, XMFLOAT3 gravity, int collisionFlags);
	//calcualte collision box from the model vertecies.
	btCollisionShape* CreateCollisionShape();
	btCylinderShapeX* CreateCollisionShapeCylinder();

	//Calculate initial position based on the offset
	btTransform GetLocalTranslation();

	void SetPosition(XMFLOAT3 position);
	void SetRotationOrigin(XMFLOAT3 rotOffset);
	void SetRotation(XMFLOAT3 rot);

	float GetWeight();
	void SetWeight(float _mass);

	//the world is setup in game, from here passed to all the objects.
	btDynamicsWorld*	m_dynamicsWorld;
protected:


	//quat rotataion to ofset the initial rotation
	XMFLOAT4 m_quatRotation;

	//extens used to construct the collsion box///half
	XMFLOAT3 m_collisionBoxExtents;

	
	btScalar		m_defaultContactProcessingThreshold;

	XMFLOAT3 m_rotationOrigin;
	float			m_mass;
};

#endif

