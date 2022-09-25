#pragma once
#include "src/Core/Core.h"
#include "src/Core/Maths.h"

namespace Nuake
{
	class Entity;

	namespace Physics
	{
		class CharacterController
		{
		public:
			bool IsOnGround = false;
			bool m_hittingWall;
			float m_stepHeight = 0.35f;
			float m_MaxSlopeAngle = 45.0f;


			//bool m_onJumpableGround; // A bit lower contact than just onGround

			float m_bottomYOffset;
			float m_bottomRoundedRegionYOffset;


			glm::vec3 m_manualVelocity;
			std::vector<glm::vec3> m_surfaceHitNormals;


			float m_jumpRechargeTimer;
			CharacterController(float height, float radius, float mass, Vector3 position);

			void SetEntity(Entity& ent);
			void MoveAndSlide(glm::vec3 velocity);

			bool IsOnFloor()
			{
				return IsOnGround;
			}

		private:
			void ParseGhostContacts();
			void UpdatePosition();
			void UpdateVelocity();
		};
	}
}
