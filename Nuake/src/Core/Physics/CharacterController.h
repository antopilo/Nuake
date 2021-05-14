#pragma once
#include <glm/ext/vector_float3.hpp>
#include "../Core/Core.h"
#include <btBulletDynamicsCommon.h>

#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <vector>
namespace Physics
{
	class CharacterController
	{
	public:
		bool IsOnGround = false;
		bool m_hittingWall;
		float m_stepHeight = 0.05f;

		btTransform* m_Transform;
		btCollisionShape* m_CollisionShape;
		btRigidBody* m_Rigidbody;
		btPairCachingGhostObject* m_GhostObject;
		btMotionState* m_MotionState;
		bool m_onGround;
		//bool m_onJumpableGround; // A bit lower contact than just onGround

		float m_bottomYOffset;
		float m_bottomRoundedRegionYOffset ;

		btTransform m_motionTransform;

		glm::vec3 m_manualVelocity;
		std::vector<glm::vec3> m_surfaceHitNormals;

		btVector3 m_previousPosition;

		float m_jumpRechargeTimer;
		CharacterController(float height, float radius, float mass);

		void MoveAndSlide(glm::vec3 velocity);
	private:
		void ParseGhostContacts();
		void UpdatePosition();
		void UpdateVelocity();
	};
}