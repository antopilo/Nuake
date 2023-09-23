#pragma once
#include "src/Core/Core.h"

#include <mutex>
#include <thread>

namespace SoLoud
{
	class Soloud;
	class AudioSource;
}

// Temp code
struct AudioRequest
{
	std::string audioFile;
	float volume = 1.0f;
	float pan = 0.0f;
	float speed = 1.0f;

};

namespace SoLoud
{
	class Wav;
}

namespace Nuake
{
	// This is a singleton that manages everything for audio.
	class AudioManager
	{
	private:
		const int MAX_VOICE_COUNT = 32;

		Ref<SoLoud::Soloud> m_Soloud;

		bool m_AudioThreadRunning;
		std::thread m_AudioThread;
		std::mutex m_AudioQueueMutex;
		std::atomic<bool> m_AudioQueued = { false };
		std::queue<AudioRequest> m_AudioQueue;

		std::unordered_map<std::string, SoLoud::Wav> m_WavSamples;
	public:
		AudioManager();
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

		void QueueWavAudio(const AudioRequest& request);
		bool IsWavLoaded(const std::string& filePath) const;
		void LoadWavAudio(const std::string& filePath);
	private:
		void AudioThreadLoop();
	};
}
