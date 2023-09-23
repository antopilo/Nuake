#pragma once
#include "src/Core/Core.h"
#include "src/Resource/Serializable.h"

namespace Nuake {

	class AudioEmitterComponent
	{
	public:
		float Volume = 1.0f;
		float Pan = 0.0f;
		float PlaybackSpeed = 1.0f;
		bool IsPlaying = false;
		std::string FilePath;

		json Serialize();
		bool Deserialize(const json& j);
	};
}
