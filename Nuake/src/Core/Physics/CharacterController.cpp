#include "CharacterController.h"
#include "src/Core/Physics/PhysicsManager.h"
#include "src/Core/Physics/RaycastResult.h"

namespace Nuake
{
	namespace Physics
	{
		CharacterController::CharacterController(const Ref<PhysicShape>& shape, float friction, float maxSlopeAngle)
		{
			Shape = shape;
			Friction = friction;
			MaxSlopeAngle = maxSlopeAngle;
		}

		void CharacterController::SetEntity(Entity& ent)
		{
			Owner = ent;
		}

		Entity CharacterController::GetEntity() const
		{
			return Owner;
		}

		void CharacterController::MoveAndSlide(glm::vec3 velocity)
		{
			IsOnGround = false;
			ParseGhostContacts();
			UpdatePosition();
			UpdateVelocity();
		}

		void CharacterController::ParseGhostContacts()
		{
			
		}

		void CharacterController::UpdatePosition()
		{
			

		}

		void CharacterController::UpdateVelocity()
		{
			

			// Decelerate
			//m_manualVelocity -= m_manualVelocity * m_deceleration * m_pPhysicsWorld->GetScene()->m_frameTimer.GetTimeMultiplier();

			if (m_hittingWall)
			{
				for (size_t i = 0, size = m_surfaceHitNormals.size(); i < size; i++)
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

