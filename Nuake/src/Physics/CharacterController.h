#pragma once
#include "src/Core/Core.h"
#include "src/Core/Maths.h"
#include "src/Physics/PhysicsShapes.h"
#include "src/Scene/Entities/Entity.h"

namespace Nuake
{
	class Entity;

	namespace Physics
	{
		struct CharacterControllerSettings
		{
			Ref<Physics::PhysicShape> Shape;
			float Friction;
			float MaxSlopeAngle;
			bool AutoStepping;
			Vector3 StepDown;
			Vector3 StepDownExtra;
			Vector3 StepUp;
			float StepMinDistance;
			float StepDistance;
		};

		class CharacterController
		{
		public:
			Entity Owner;

			// Settings
			Ref<PhysicShape> Shape;
			float Friction = 0.5f;
			float MaxSlopeAngle = 45.0f;
			bool AutoStepping;
			Vector3 StepDown;
			Vector3 StepDownExtra;
			Vector3 StepUp;
			float StepDistance;
			float StepMinDistance;

			// State
			Vector3 Position = Vector3(0, 0, 0);
			Quat Rotation = Quat(1, 0, 0, 0);
			bool IsOnGround = false;

		public:
			CharacterController(const CharacterControllerSettings& settings);

			void MoveAndSlide(const Vector3& velocity);

			void SetEntity(Entity& ent);
			Entity GetEntity() const;

			bool IsOnFloor() const
			{
				return IsOnGround;
			}
		};
	}
}
