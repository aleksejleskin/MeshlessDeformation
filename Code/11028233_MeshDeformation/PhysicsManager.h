#ifndef PHYSICS_MANAGER_H
#define PHYSICS_MANAGER_H

#include "btBulletDynamicsCommon.h"
#include "MathHelper.h"
#include <iostream>

#include <Windows.h>
#include <string>


class PhysicsManager
{
public:
	PhysicsManager();
	~PhysicsManager();

	bool LoadContent();
	void Update(float dt);


	//create a rigid body and add to the simualtion world.
	btRigidBody* localCreateRigidBody(float mass, const btTransform& startTransform, btCollisionShape* shape);

	btDynamicsWorld* GetWorld();
private:
	//main pointer to the physics simualtion.
	btDynamicsWorld*		 m_dynamicsWorld;
	//set up constraints for bullet physics initialization
	btBroadphaseInterface*			 m_broadphase;
	btCollisionDispatcher*			 m_dispatcher;
	btConstraintSolver*				 m_solver;
	btDefaultCollisionConfiguration* m_collisionConfiguration;
	btScalar						 m_defaultContactProcessingThreshold;
};

#endif