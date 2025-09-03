#pragma once

#include "Component.h"

#include "Nuake/Physics/CharacterController.h"

namespace Nuake 
{
	class CharacterControllerComponent : public Component
	{
		NUAKECOMPONENT(CharacterControllerComponent, "Character Controller")

	private:
		Ref<Physics::CharacterController>  m_CharacterController;

	public:
		// Auto stepping settings
		bool AutoStepping = false;
		Vector3 StickToFloorStepDown = Vector3(0.f, -0.5f, 0.f);
		Vector3 StepDownExtra = Vector3(0, 0, 0); // ??
		Vector3 SteppingStepUp = Vector3(0.f, 0.4f, 0.f);
		float SteppingMinDistance = 0.125f;
		float SteppingForwardDistance = 0.250f;
		
		float Friction = 0.5f;
		float MaxSlopeAngle = 45.0f;
		float MaxStrength = 1.0f;

		CharacterControllerComponent()
		{
		}

		void SetCharacterController(const Ref<Physics::CharacterController>& charController)
		{
			m_CharacterController = charController;
		}

		Ref<Physics::CharacterController> GetCharacterController() const 
		{
			return m_CharacterController;
		}

		json Serialize() 
		{
			BEGIN_SERIALIZE();
			SERIALIZE_VAL(AutoStepping);
			SERIALIZE_VEC3(StickToFloorStepDown);
			SERIALIZE_VEC3(StepDownExtra);
			SERIALIZE_VEC3(SteppingStepUp);
			SERIALIZE_VAL(SteppingMinDistance);
			SERIALIZE_VAL(SteppingForwardDistance);

			SERIALIZE_VAL(Friction);
			SERIALIZE_VAL(MaxSlopeAngle);
			SERIALIZE_VAL(MaxStrength);
			END_SERIALIZE();
		}

		bool Deserialize(const json& j) 
		{
			DESERIALIZE_VAL(AutoStepping);
			DESERIALIZE_VAL(Friction);
			DESERIALIZE_VAL(MaxSlopeAngle);
			
			if (j.contains("StickToFloorStepDown"))
			{
				DESERIALIZE_VEC3(j["StickToFloorStepDown"], StickToFloorStepDown);
			}

			if (j.contains("StepDownExtra"))
			{
				DESERIALIZE_VEC3(j["StepDownExtra"], StepDownExtra);
			}

			if (j.contains("SteppingStepUp"))
			{
				DESERIALIZE_VEC3(j["SteppingStepUp"], SteppingStepUp);
			}

			DESERIALIZE_VAL(SteppingMinDistance);
			DESERIALIZE_VAL(SteppingForwardDistance);

			DESERIALIZE_VAL(Friction);
			DESERIALIZE_VAL(MaxSlopeAngle);
			DESERIALIZE_VAL(MaxStrength);
			return true;
		}
	};
}
