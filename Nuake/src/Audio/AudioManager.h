#pragma once
#include "src/Core/Core.h"

#include <mutex>
#include <thread>

namespace SoLoud
{
	class Soloud;
	class AudioSource;
}

namespace Nuake
{
	// This is a singleton that manages everything for audio.
	class AudioManager
	{
	private:
		Ref<SoLoud::Soloud> _soloud;

		bool _audioThreadRunning;
		std::thread _audioThread;
		std::mutex _audioMutex;

		const int MAX_VOICE_COUNT = 32;
	public:
		AudioManager() = default;
		~AudioManager();

		static AudioManager& Get()
		{
			static AudioManager instance;
			return instance;
		}

		void Initialize();
		void Deinitialize();
		
		// Volume
		float GetVolume() const;
		void SetVolume(float volume);

		void PlayTTS(const std::string& text);

	private:
		void AudioThreadLoop();
	};
}
