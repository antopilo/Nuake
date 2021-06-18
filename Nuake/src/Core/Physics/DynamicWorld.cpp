#include "DynamicWorld.h"
#include "Rigibody.h"
#include "../Core/Core.h"
#include "BulletDebugDrawer.h"
#include <src/Vendors/glm/ext/quaternion_common.hpp>
#include <src/Core/Logger.h>

namespace Physics
{

	DynamicWorld::DynamicWorld() {
		///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
		btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();

		///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
		btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);

		///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
		btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();

		///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
		btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;

		dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
		dynamicsWorld->setDebugDrawer(new BulletDebugDrawer());

		m_Bodies = std::map<btRigidBody*, Ref<RigidBody>>();

		SetGravity(Vector3(0, -10000, 0));
	}


	void DynamicWorld::DrawDebug()
	{
		dynamicsWorld->debugDrawWorld();
	}


	void DynamicWorld::SetGravity(glm::vec3 g)
	{
		dynamicsWorld->setGravity(btVector3(g.x, g.y, g.z));
	}


	void DynamicWorld::AddRigidbody(Ref<RigidBody> rb)
	{
		btRigidBody* bt = rb->GetBulletRigidbody();
		m_Bodies.emplace(std::pair<btRigidBody*, Ref<RigidBody>>(bt, rb));
		dynamicsWorld->addRigidBody(rb->GetBulletRigidbody());
	}

	void DynamicWorld::AddCharacterController(Ref<CharacterController> cc)
	{
		dynamicsWorld->addRigidBody(cc->m_Rigidbody);

		// Specify filters manually, otherwise ghost doesn't collide with statics for some reason
		dynamicsWorld->addCollisionObject(cc->m_GhostObject, btBroadphaseProxy::KinematicFilter, btBroadphaseProxy::StaticFilter );
	}
	

	RaycastResult DynamicWorld::Raycast(glm::vec3 from, glm::vec3 to)
	{
		btVector3 btFrom(from.x, from.y, from.z);
		btVector3 btTo(to.x, to.y, to.z);
		ClosestRayResultCallback res(btFrom, btTo);

		dynamicsWorld->rayTest(btFrom, btTo, res);
		btVector3 localNormal;
		if(res.m_collisionObject)
		{ 
			// TODO: Fix the godammn fucked up normal
			localNormal = res.m_hitNormalWorld;
		}
		
		
		Vector3 localNorm = glm::vec3(localNormal.x(), localNormal.y(), localNormal.z());

		//Logger::Log("normal: x:" + std::to_string(localNorm.x) + " y:" + std::to_string(localNorm.y )+ "z: " + std::to_string(localNorm.z));
		res.m_closestHitFraction;
		// Map bullet result to dto.
		RaycastResult result{
			glm::vec3(res.m_hitPointWorld.x(), res.m_hitPointWorld.y(), res.m_hitPointWorld.z()),
			glm::vec3(res.m_hitPointWorld.x(), res.m_hitPointWorld.y(), res.m_hitPointWorld.z()),
			localNorm
		};

		return result;
	}


	void DynamicWorld::StepSimulation(Timestep ts)
	{
		dynamicsWorld->stepSimulation(ts, 10);
		for (int j = dynamicsWorld->getNumCollisionObjects() - 1; j >= 0; j--)
		{
			btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[j];
			btRigidBody* body = btRigidBody::upcast(obj);
			btTransform trans;
			if (body && body->getMotionState())
			{
			
				body->getMotionState()->getWorldTransform(trans);
				if(m_Bodies.find(body) != m_Bodies.end())
					m_Bodies[body]->UpdateTransform(trans);
			}
			else
			{
				trans = obj->getWorldTransform();
				if (m_Bodies.find(body) != m_Bodies.end())
					m_Bodies[body]->UpdateTransform(trans);
			}
			//printf("world pos object %d = %f,%f,%f\n", j, float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ()));
		}
	}


	void DynamicWorld::Clear()
	{
		for (int j = dynamicsWorld->getNumCollisionObjects() - 1; j >= 0; j--)
		{
			btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[j];
			dynamicsWorld->removeCollisionObject(obj);
			printf("Object cleared %d\n", j);
		}

		m_Bodies.clear();
	}
}
