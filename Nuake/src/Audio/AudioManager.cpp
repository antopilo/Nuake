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
		SoLoud::Speech speech;
		speech.setText(text.c_str());
		_soloud->play(speech);

		// Wait until sounds have finished
		while (_soloud->getActiveVoiceCount() > 0)
		{
			// Still going, sleep for a bit
			SoLoud::Thread::sleep(100);
		}
	}

	void AudioManager::AudioThreadLoop()
	{
		while(_audioThreadRunning)
		{
			
		}
	}
}