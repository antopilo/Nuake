#include "CharacterController.h"
#include "Nuake/Physics/PhysicsManager.h"
#include "Nuake/Physics/RaycastResult.h"

#include "Nuake/Physics/PhysicsManager.h"

namespace Nuake
{
	namespace Physics
	{
		CharacterController::CharacterController(const CharacterControllerSettings& settings)
		{
			Shape = settings.Shape;
			Friction = settings.Friction;
			MaxSlopeAngle = settings.MaxSlopeAngle;
			AutoStepping = settings.AutoStepping;
			StepDown = settings.StepDown;
			StepDownExtra = settings.StepDownExtra;
			StepUp = settings.StepUp;
			StepDistance = settings.StepDistance;
			StepMinDistance = settings.StepMinDistance;
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

