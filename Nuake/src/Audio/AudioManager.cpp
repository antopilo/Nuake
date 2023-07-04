#include "AudioManager.h"

#include <soloud.h>
#include "soloud_speech.h"
#include "soloud_thread.h"

namespace Nuake
{
	void AudioManager::Initialize()
	{
		_soloud = CreateRef<SoLoud::Soloud>();
		_soloud->init();

		_audioThreadRunning = false;
		_audioThread = std::thread(&AudioManager::AudioThreadLoop, this);
	}

	AudioManager::~AudioManager()
	{
		Deinitialize();

		_audioThreadRunning = false;
		_audioThread.join();
	}


	void AudioManager::Deinitialize()
	{
		_soloud->deinit();
	}

	void AudioManager::PlayTTS(const std::string& text)
	{

	}

	void AudioManager::AudioThreadLoop()
	{
		while(_audioThreadRunning)
		{
			
		}
	}
}