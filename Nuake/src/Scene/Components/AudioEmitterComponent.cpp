#include "AudioEmitterComponent.h"

namespace Nuake {
	json AudioEmitterComponent::Serialize()
	{
		BEGIN_SERIALIZE();
		SERIALIZE_VAL(FilePath);
		SERIALIZE_VAL(Volume);
		SERIALIZE_VAL(Pan);
		SERIALIZE_VAL(PlaybackSpeed);
		END_SERIALIZE();
	}

	bool AudioEmitterComponent::Deserialize(const json& j)
	{
		DESERIALIZE_VAL(FilePath);
		DESERIALIZE_VAL(Volume);
		DESERIALIZE_VAL(Pan);
		DESERIALIZE_VAL(PlaybackSpeed);
		return true;
	}
}