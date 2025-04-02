#include "AudioModule.h"
#include "AudioSceneSystem.h"
#include "Nuake/Core/Maths.h"
#include "Nuake/Scene/SceneSystems.h"
#include "Nuake/Modules/ModuleDB.h"
#include <Nuake/Core/Logger.h>

#include "Nuake/Audio/AudioManager.h"

void PlayAudio(int id, Nuake::Matrix4 volume)
{

}

void StopAudio()
{

}

float Volume = 1.0f;
void SetVolume(float volume)
{
	Volume = volume;
}

bool Muted = false;
void SetMuted(bool muted)
{
	Muted = muted;
}

NUAKEMODULE(AudioModule)
void AudioModule_Startup()
{
	using namespace Nuake;

	auto& module = ModuleDB::Get().RegisterModule<AudioModule>();
	module.Name = "AudioModule";
	module.Description = "Core audio module";

	module.RegisterSetting<&Volume>("Volume");
	module.RegisterSetting<&Muted>("Muted");

	module.BindFunction<PlayAudio>("PlayAudio", "id", "volume");
	module.BindFunction<StopAudio>("StopAudio");

	AudioManager::Get().Initialize();

	SceneSystemDB::Get().RegisterSceneSystem<Audio::AudioSystem>();

	module.OnUpdate.AddStatic([](float ts) 
	{
		auto& audioMgr = AudioManager::Get();
		audioMgr.SetGlobalVolume(Volume);

		audioMgr.AudioUpdate();
	});
}

void AudioModule_Shutdown()
{

}