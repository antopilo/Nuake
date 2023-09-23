#include "AudioManager.h"
#include "src/Core/Logger.h"
#include "src/Core/FileSystem.h"

#include <soloud.h>
#include "soloud_speech.h"
#include "soloud_thread.h"
#include <dependencies/soloud/include/soloud_wav.h>


namespace Nuake {

	AudioManager::AudioManager() : 
		m_AudioThreadRunning(true)
	{
	}

	AudioManager::~AudioManager()
	{
		Deinitialize();

		m_AudioThreadRunning = false;
		m_AudioThread.join();
	}

	void AudioManager::Initialize()
	{
		m_Soloud = CreateRef<SoLoud::Soloud>();

		// TODO: Sample rate, back end, buffer size, flags.
		m_Soloud->init();

		m_AudioThread = std::thread(&AudioManager::AudioThreadLoop, this);

		Logger::Log("Audio manager initialized", "audio", VERBOSE);
	}

	void AudioManager::Deinitialize()
	{
		m_Soloud->deinit();
	}

	void AudioManager::PlayTTS(const std::string& text)
	{

	}

	void AudioManager::QueueWavAudio(const AudioRequest& request)
	{
		// Acquire mutex lock and push to queue
		const std::lock_guard<std::mutex> lock(m_AudioQueueMutex);

		// Check if file exists and load
		const bool fileExists = FileSystem::FileExists(request.audioFile, true);
		if (fileExists && !IsWavLoaded(request.audioFile))
		{
			LoadWavAudio(request.audioFile);
		}

		m_AudioQueue.push(request);
	}

	bool AudioManager::IsWavLoaded(const std::string& filePath) const
	{
		return m_WavSamples.find(filePath) != m_WavSamples.end();
	}

	void AudioManager::LoadWavAudio(const std::string& filePath)
	{
		m_WavSamples[filePath] = SoLoud::Wav();
		m_WavSamples[filePath].load(filePath.c_str());
	}

	void AudioManager::AudioThreadLoop()
	{
		while(m_AudioThreadRunning)
		{
			// Acquire mutex lock
			const std::lock_guard<std::mutex> lock(m_AudioQueueMutex);

			// Check if we have audio queued
			while (!m_AudioQueue.empty())
			{
				AudioRequest& currentAudio = m_AudioQueue.front();

				SoLoud::handle soloudHandle = m_Soloud->play(m_WavSamples[currentAudio.audioFile]);
				m_Soloud->setVolume(soloudHandle, currentAudio.volume);
				m_Soloud->setPan(soloudHandle, currentAudio.pan);
				m_Soloud->setRelativePlaySpeed(soloudHandle, currentAudio.speed);

				// Remove item from queue.
				m_AudioQueue.pop();
			}
		}
	}
}