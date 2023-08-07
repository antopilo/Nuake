#include "src/Scene/Components/ParticleEmitterComponent.h"

namespace Nuake
{
	json ParticleEmitterComponent::Serialize()
	{
		BEGIN_SERIALIZE();
		SERIALIZE_VEC4(ParticleColor);
		SERIALIZE_VAL(Amount);
		SERIALIZE_VAL(Life);
		SERIALIZE_VAL(Rate);
		SERIALIZE_VEC3(Gravity);
		SERIALIZE_VAL(GravityRandom);
		SERIALIZE_VAL(Radius);
		END_SERIALIZE();
	}

	bool ParticleEmitterComponent::Deserialize(const json& j)
	{
		DESERIALIZE_VEC4(j["ParticleColor"], ParticleColor);
		Amount = j["Amount"];
		Life = j["Life"];
		if (j.contains("Rate"))
		{
			Rate = j["Rate"];
		}
		DESERIALIZE_VEC3(j["Gravity"], Gravity);
		GravityRandom = j["GravityRandom"];
		Radius = j["Radius"];
		return true;
	}
}