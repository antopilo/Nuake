#pragma once
#include "src/Core/Physics/CharacterController.h"

namespace Nuake {
	class CharacterControllerComponent
	{
	public:
		Ref<Physics::CharacterController>  CharacterController;

		float Friction = 0.5f;
		float MaxSlopeAngle = 0.45f;

		CharacterControllerComponent()
		{

		}

		json Serialize() 
		{
			BEGIN_SERIALIZE();
			SERIALIZE_VAL(Friction);
			SERIALIZE_VAL(MaxSlopeAngle);
			END_SERIALIZE();
		}

		bool Deserialize(const std::string str) 
		{
			BEGIN_DESERIALIZE();

			if (j.contains("Friction"))
			{
				Friction = j["Friction"];
			}
			
			if (j.contains("MaxSlopeAngle"))
			{
				MaxSlopeAngle = j["MaxSlopeAngle"];
			}

			return true;
		}
	};
}
