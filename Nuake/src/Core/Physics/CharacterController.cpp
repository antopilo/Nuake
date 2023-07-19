#include "CharacterController.h"
#include "src/Core/Physics/PhysicsManager.h"
#include "src/Core/Physics/RaycastResult.h"

#include "src/Core/Physics/PhysicsManager.h"

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

		void CharacterController::MoveAndSlide(const Vector3& velocity)
		{
			PhysicsManager::Get().GetWorld()->MoveAndSlideCharacterController(Owner, velocity);
		}
	}
}

