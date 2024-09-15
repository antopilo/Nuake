#include "src/Scene/Components/ParticleEmitterComponent.h"
#include "src/FileSystem/File.h"
#include <src/Resource/ResourceManager.h>
#include <src/Resource/ResourceLoader.h>

namespace Nuake
{
	json ParticleEmitterComponent::Serialize()
	{
		BEGIN_SERIALIZE();
		SERIALIZE_RES_FILE(resFile);
		SERIALIZE_VEC3(ParticleScale);
		SERIALIZE_VAL(Amount);
		SERIALIZE_VAL(Life);
		SERIALIZE_VAL(Rate);
		SERIALIZE_VEC3(Gravity);
		SERIALIZE_VAL(GravityRandom);
		SERIALIZE_VAL(Radius);
		SERIALIZE_VAL(GlobalSpace);
		SERIALIZE_VAL(LifeRandomness);
		SERIALIZE_VAL(ScaleRandomness);
		END_SERIALIZE();
	}

	bool ParticleEmitterComponent::Deserialize(const json& j)
	{
		Amount = j["Amount"];
		Life = j["Life"];

		if (j.contains("LifeRandomness"))
		{
			LifeRandomness = j["LifeRandomness"];
		}

		if (j.contains("ScaleRandomness"))
		{
			ScaleRandomness = j["ScaleRandomness"];
		}

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
		GravityRandom = j["GravityRandom"];
		Radius = j["Radius"];

		DESERIALIZE_RES_FILE(resFile);
		return true;
	}
}
