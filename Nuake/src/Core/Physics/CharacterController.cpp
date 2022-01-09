#include "CharacterController.h"
#include "src/Core/Physics/PhysicsManager.h"
#include "src/Core/Physics/RaycastResult.h"

namespace Nuake
{
	namespace Physics
	{
		CharacterController::CharacterController(float height, float radius, float mass, Vector3 position)
		{
			m_surfaceHitNormals = std::vector<glm::vec3>();

			m_bottomRoundedRegionYOffset = (height + radius) / 2.0f;
			m_bottomYOffset = height / 2.0f + radius;
			m_CollisionShape = new btCapsuleShape(radius, height);

			btQuaternion quat = btQuaternion(0, 0, 0);
			m_Transform = new btTransform();
			m_Transform->setOrigin(btVector3(position.x, position.y, position.z));
			m_Transform->setRotation(quat);
			m_MotionState = new btDefaultMotionState(*m_Transform);

			btVector3 inertia;
			//m_CollisionShape->calculateLocalInertia(mass, inertia);

			btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, m_MotionState, m_CollisionShape, inertia);

			rigidBodyCI.m_friction = 0.0f;
			//rigidBodyCI.m_additionalDamping = true;
			//rigidBodyCI.m_additionalLinearDampingThresholdSqr= 1.0f;
			//rigidBodyCI.m_additionalLinearDampingThresholdSqr = 0.5f;
			rigidBodyCI.m_restitution = 0.0f;

			rigidBodyCI.m_linearDamping = 0.0f;
			m_Rigidbody = new btRigidBody(rigidBodyCI);
			m_Rigidbody->setGravity(btVector3(0, 0, 0));
			// Keep upright
			m_Rigidbody->setAngularFactor(0.0f);

			// No sleeping (or else setLinearVelocity won't work)
			m_Rigidbody->setActivationState(DISABLE_DEACTIVATION);

			//m_pPhysicsWorld->m_pDynamicsWorld->addRigidBody(m_pRigidBody);

			// Ghost object that is synchronized with rigid body
			m_GhostObject = new btPairCachingGhostObject();
			m_GhostObject->setCollisionShape(m_CollisionShape);

			// Specify filters manually, otherwise ghost doesn't collide with statics for some reason
			//m_pPhysicsWorld->m_pDynamicsWorld->addCollisionObject(m_pGhostObject, btBroadphaseProxy::KinematicFilter, btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);
		}

		void CharacterController::SetEntity(Entity& ent)
		{
			m_Rigidbody->setUserIndex(ent.GetHandle());
			m_GhostObject->setUserIndex(ent.GetHandle());
		}



		void CharacterController::MoveAndSlide(glm::vec3 velocity)
		{
			m_Rigidbody->setGravity(btVector3(0, 0, 0));
			m_manualVelocity = velocity;
			// Sync ghost with actually object
			m_GhostObject->setWorldTransform(m_Rigidbody->getWorldTransform());

			IsOnGround = false;
			ParseGhostContacts();
			UpdatePosition();
			UpdateVelocity();
			m_MotionState->getWorldTransform(m_motionTransform);
		}

		void CharacterController::ParseGhostContacts()
		{
			btManifoldArray manifoldArray;
			btBroadphasePairArray& pairArray = m_GhostObject->getOverlappingPairCache()->getOverlappingPairArray();
			int numPairs = pairArray.size();

			m_hittingWall = false;

			for (int i = 0; i < numPairs; i++)
			{
				manifoldArray.clear();

				const btBroadphasePair& pair = pairArray[i];

				btDiscreteDynamicsWorld* world = PhysicsManager::Get()->GetWorld()->GetDynamicWorld();
				btBroadphasePair* collisionPair = world->getPairCache()->findPair(pair.m_pProxy0, pair.m_pProxy1);

				if (collisionPair == NULL)
					continue;

				if (collisionPair->m_algorithm != NULL)
					collisionPair->m_algorithm->getAllContactManifolds(manifoldArray);

				for (int j = 0; j < manifoldArray.size(); j++)
				{
					btPersistentManifold* pManifold = manifoldArray[j];

					// Skip the rigid body the ghost monitors
					if (pManifold->getBody0() == m_Rigidbody)
						continue;

					for (int p = 0; p < pManifold->getNumContacts(); p++)
					{
						const btManifoldPoint& point = pManifold->getContactPoint(p);

						if (point.getDistance() < 0.0f)
						{
							//const btVector3 &ptA = point.getPositionWorldOnA();
							const btVector3& ptB = point.getPositionWorldOnB();

							//const btVector3 &normalOnB = point.m_normalWorldOnB;

							// If point is in rounded bottom region of capsule shape, it is on the ground
							if (ptB.getY() < m_motionTransform.getOrigin().getY() - m_bottomRoundedRegionYOffset)
								IsOnGround = true;
							else
							{
								m_hittingWall = true;

								m_surfaceHitNormals.push_back(glm::vec3(point.m_normalWorldOnB.x(), point.m_normalWorldOnB.y(), point.m_normalWorldOnB.z()));
							}
						}
					}
				}
			}
		}

		void CharacterController::UpdatePosition()
		{
			// Ray cast, ignore rigid body
			IgnoreBodyAndGhostCast rayCallBack_bottom(m_Rigidbody, m_GhostObject);

			btVector3 from = m_Rigidbody->getWorldTransform().getOrigin();
			btVector3 toBt = m_Rigidbody->getWorldTransform().getOrigin() - btVector3(0.0f, m_bottomYOffset + m_stepHeight, 0.0f);
			PhysicsManager::Get()->GetWorld()->GetDynamicWorld()->rayTest(from, toBt, rayCallBack_bottom);

			// Bump up if hit
			if (rayCallBack_bottom.hasHit())
			{
				float previousY = m_Rigidbody->getWorldTransform().getOrigin().getY();


				float t = rayCallBack_bottom.m_closestHitFraction;

				float clamped = (1.0 - rayCallBack_bottom.m_closestHitFraction);
				btVector3 vel(m_Rigidbody->getLinearVelocity());

				if (vel.getY() < 0) // -magic number is to fix bouncing down a slope.
					m_Rigidbody->getWorldTransform().getOrigin().setY(previousY - 0.14f + (m_bottomYOffset + m_stepHeight) * (clamped));

				vel.setY(0.0f);

				m_Rigidbody->setLinearVelocity(vel);

				IsOnGround = true;

			}



			float testOffset = 0.07f;

			// Ray cast, ignore rigid body
			IgnoreBodyAndGhostCast rayCallBack_top(m_Rigidbody, m_GhostObject);

			PhysicsManager::Get()->GetWorld()->GetDynamicWorld()->rayTest(m_Rigidbody->getWorldTransform().getOrigin(), m_Rigidbody->getWorldTransform().getOrigin() + btVector3(0.0f, m_bottomYOffset + testOffset, 0.0f), rayCallBack_top);

			// Bump up if hit
			if (rayCallBack_top.hasHit())
			{
				m_Rigidbody->getWorldTransform().setOrigin(m_previousPosition);

				btVector3 vel(m_Rigidbody->getLinearVelocity());

				vel.setY(0.0f);

				m_Rigidbody->setLinearVelocity(vel);
			}

			m_previousPosition = m_Rigidbody->getWorldTransform().getOrigin();
		}

		void CharacterController::UpdateVelocity()
		{
			//m_manualVelocity.y = m_Rigidbody->getLinearVelocity().getY();
			btVector3 grav = m_Rigidbody->getGravity();
			btVector3 finalVel = btVector3(m_manualVelocity.x, m_manualVelocity.y, m_manualVelocity.z);
			m_Rigidbody->setLinearVelocity(finalVel);

			// Decelerate
			//m_manualVelocity -= m_manualVelocity * m_deceleration * m_pPhysicsWorld->GetScene()->m_frameTimer.GetTimeMultiplier();

			if (m_hittingWall)
			{
				for (unsigned int i = 0, size = m_surfaceHitNormals.size(); i < size; i++)
				{
					// Cancel velocity across normal
					glm::vec3 velInNormalDir(glm::reflect(m_manualVelocity, m_surfaceHitNormals[i]));

					// Apply correction
					m_manualVelocity -= velInNormalDir * 1.05f;
				}

				// Do not adjust rigid body velocity manually (so bodies can still be pushed by character)
				return;
			}
		}
	}
}

