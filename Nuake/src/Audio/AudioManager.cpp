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

		_audioThreadRunning = true;
		_audioThread = std::thread(&AudioManager::AudioThreadLoop, this);
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
		SoLoud::Speech speech;
		speech.setText("Nuake");
		_soloud->play(speech);
		while(_audioThreadRunning)
		{
			
		}
	}
}