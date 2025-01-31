#include "AudioEmitterComponent.h"

#include "Nuake/FileSystem/File.h"
#include "Nuake/FileSystem/FileSystem.h"

namespace Nuake {

	json AudioEmitterComponent::Serialize()
	{
		BEGIN_SERIALIZE();
		SERIALIZE_RES_FILE(FilePath);
		SERIALIZE_VAL(IsPlaying);
		SERIALIZE_VAL(Volume);
		SERIALIZE_VAL(Pan);
		SERIALIZE_VAL(PlaybackSpeed);
		SERIALIZE_VAL(Spatialized);
		SERIALIZE_VAL(MinDistance);
		SERIALIZE_VAL(MaxDistance);
		SERIALIZE_VAL(AttenuationFactor);
		SERIALIZE_VAL(Loop);
		END_SERIALIZE();
	}

	bool AudioEmitterComponent::Deserialize(const json& j)
	{
		DESERIALIZE_RES_FILE(FilePath);
		DESERIALIZE_VAL(Volume);
		DESERIALIZE_VAL(IsPlaying);
		DESERIALIZE_VAL(Pan);
		DESERIALIZE_VAL(PlaybackSpeed);
		DESERIALIZE_VAL(Spatialized);
		DESERIALIZE_VAL(MinDistance);
		DESERIALIZE_VAL(MaxDistance);
		DESERIALIZE_VAL(AttenuationFactor);
		DESERIALIZE_VAL(Loop);
		return true;
	}
}
