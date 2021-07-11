#pragma once
#include <glm/ext/vector_float3.hpp>
#include "CharacterController.h"

#include <BulletCollision/CollisionDispatch/btGhostObject.h>

namespace Nuake
{
	namespace Physics
	{
		struct ClosestRayResultCallback : public btCollisionWorld::RayResultCallback
		{
			ClosestRayResultCallback(const btVector3& rayFromWorld, const btVector3& rayToWorld)
				: m_rayFromWorld(rayFromWorld),
				m_rayToWorld(rayToWorld)
			{
			}

			btVector3 m_rayFromWorld;  //used to calculate hitPointWorld from hitFraction
			btVector3 m_rayToWorld;

			btVector3 m_hitNormalWorld;
			btVector3 m_hitPointWorld;

			virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
			{
				//caller already does the filter on the m_closestHitFraction
				btAssert(rayResult.m_hitFraction <= m_closestHitFraction);

				m_closestHitFraction = rayResult.m_hitFraction;
				m_collisionObject = rayResult.m_collisionObject;

				m_hitNormalWorld = rayResult.m_hitNormalLocal;


				m_hitPointWorld.setInterpolate3(m_rayFromWorld, m_rayToWorld, rayResult.m_hitFraction);
				return rayResult.m_hitFraction;
			}
		};

		class IgnoreBodyAndGhostCast :
			public Physics::ClosestRayResultCallback
		{
		private:
			btRigidBody* m_pBody;
			btPairCachingGhostObject* m_pGhostObject;

		public:
			IgnoreBodyAndGhostCast(btRigidBody* pBody, btPairCachingGhostObject* pGhostObject)
				: ClosestRayResultCallback(btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0)),
				m_pBody(pBody), m_pGhostObject(pGhostObject)
			{
			}

			btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
			{
				if (rayResult.m_collisionObject == m_pBody || rayResult.m_collisionObject == m_pGhostObject)
					return 1.0f;

				return ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);
			}
		};
	}


	// result object from raycast.
	struct RaycastResult {
		glm::vec3 WorldPoint;
		glm::vec3 LocalPoint;
		glm::vec3 Normal;
	};
}
