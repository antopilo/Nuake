#include "src/Scene/Components/ParticleEmitterComponent.h"

namespace Nuake
{
	json ParticleEmitterComponent::Serialize()
	{
		BEGIN_SERIALIZE();
		SERIALIZE_VEC4(ParticleColor);
		SERIALIZE_VEC3(ParticleScale);
		SERIALIZE_VAL(Amount);
		SERIALIZE_VAL(Life);
		SERIALIZE_VAL(Rate);
		SERIALIZE_VEC3(Gravity);
		SERIALIZE_VAL(GravityRandom);
		SERIALIZE_VAL(Radius);
		SERIALIZE_VAL(GlobalSpace);

		SERIALIZE_OBJECT(ParticleMaterial);

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
		if (j.contains("ParticleScale"))
		{
			DESERIALIZE_VEC3(j["ParticleScale"], ParticleScale);
		}
		if (j.contains("GlobalSpace"))
		{
			DESERIALIZE_VAL(GlobalSpace);
		}
		if (j.contains("ParticleMaterial"))
		{
			Ref<Material> newMaterial = CreateRef<Material>();
			newMaterial->Deserialize(j["ParticleMaterial"]);
			ParticleMaterial = newMaterial;
		}
		GravityRandom = j["GravityRandom"];
		Radius = j["Radius"];
		return true;
	}
}