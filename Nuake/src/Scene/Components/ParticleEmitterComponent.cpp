#include "src/Scene/Components/ParticleEmitterComponent.h"

namespace Nuake
{
	json ParticleEmitterComponent::Serialize()
	{
		BEGIN_SERIALIZE();
		SERIALIZE_VEC4(ParticleColor);
		SERIALIZE_VAL(Amount);
		SERIALIZE_VAL(Life);
		SERIALIZE_VEC3(Gravity);
		SERIALIZE_VAL(GravityRandom);
		SERIALIZE_VAL(Radius);
		END_SERIALIZE();
	}

	bool ParticleEmitterComponent::Deserialize(const std::string& str)
	{
		BEGIN_DESERIALIZE();
		DESERIALIZE_VEC4(j["ParticleColor"], ParticleColor);
		Amount = j["Amount"];
		Life = j["Life"];
		DESERIALIZE_VEC3(j["Gravity"], Gravity);
		GravityRandom = j["GravityRandom"];
		Radius = j["Radius"];
		return true;
	}
}