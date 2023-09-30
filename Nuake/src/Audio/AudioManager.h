#pragma once
#include "src/Core/Core.h"
#include <src/Core/Maths.h>

#include <atomic>
#include <mutex>
#include <thread>


namespace SoLoud
{
	class Soloud;
	class AudioSource;
	class Wav;
}

namespace Nuake
{
	struct AudioRequest
	{
		std::string audioFile;
		float volume = 1.0f;
		float pan = 0.0f;
		float speed = 1.0f;
		bool spatialized = false;
		Vector3 position;
		float MinDistance = 1.0f;
		float MaxDistance = 25.0f;
		float AttenuationFactor = 1.0f;
		bool Loop = false;
	};

	// This is a singleton that manages everything for audio.
	class AudioManager
	{
	private:
		const int MAX_VOICE_COUNT = 32;
		float m_GlobalVolume = 1.0f;

		Ref<SoLoud::Soloud> m_Soloud;

		bool m_AudioThreadRunning;
		std::thread m_AudioThread;
		std::mutex m_AudioQueueMutex;
		std::atomic<bool> m_AudioQueued = { false };
		std::queue<AudioRequest> m_AudioQueue;

		Vector3 m_ListenerPosition;
		Vector3 m_ListenerDirection;
		Vector3 m_ListenerUp;

		std::unordered_map<std::string, SoLoud::Wav> m_WavSamples;
		std::unordered_map<std::string, unsigned int> m_ActiveClips;

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

		void SetListenerPosition(const Vector3& position, const Vector3& direction, const Vector3& up);

		void PlayTTS(const std::string& text);

		float GetGlobalVolume() const { return m_GlobalVolume; }

		void QueueWavAudio(const AudioRequest& request);
		void UpdateVoice(const AudioRequest & request);
		void StopVoice(const std::string& filePath);
		void StopAll() const;
		bool IsWavLoaded(const std::string& filePath) const;
		bool IsVoiceActive(const std::string & voice) const;
		void LoadWavAudio(const std::string& filePath);

	private:
		void AudioThreadLoop();

		void CleanupInactiveVoices();
	};
}
