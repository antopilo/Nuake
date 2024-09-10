#pragma once

#include "src/Core/Object/Object.h"
#include "src/Core/Core.h"
#include "src/Resource/Serializable.h"

namespace Nuake {

	class AudioEmitterComponent
	{
		NUAKECOMPONENT(AudioEmitterComponent, "Audio Emitter")

	public:
		std::string FilePath;

		bool IsPlaying = false;
		bool Loop = false;

		float Volume = 1.0f;
		float Pan = 0.0f;
		float PlaybackSpeed = 1.0f;
		
		bool Spatialized = false;
		float MinDistance = 1.0f;
		float MaxDistance = 10.0f;
		float AttenuationFactor = 1.0f;

		json Serialize();
		bool Deserialize(const json& j);
	};
}
