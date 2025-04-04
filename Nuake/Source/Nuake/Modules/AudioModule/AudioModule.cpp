#include "AudioModule.h"
#include "AudioSceneSystem.h"
#include "Nuake/Core/Maths.h"
#include "Nuake/Scene/SceneSystems.h"
#include "Nuake/Modules/ModuleDB.h"
#include <Nuake/Core/Logger.h>

#include "Nuake/Audio/AudioManager.h"
#include "Nuake/Scene/Components/AudioEmitterComponent.h"

using namespace Nuake;

void HelloWorld()
{
	Logger::Log("Hello World");
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

void SetName(const std::string& name)
{
	Logger::Log("SetName: " + name);
}

NUAKEMODULE(AudioModule)
void AudioModule_Startup()
{
	// Register module info
	auto& module = ModuleDB::Get().RegisterModule<AudioModule>();
	module.Name = "AudioModule";
	module.Description = "Core audio module";

	// Exposed settings
	module.RegisterSetting<&Volume>("Volume");
	module.RegisterSetting<&Muted>("Muted");

	// Exposed functions(scripting API)
	module.BindFunction<SetVolume>("SetVolume", "volume");
	module.BindFunction<SetMuted>("SetMuted", "muted");
	module.BindFunction<HelloWorld>("HelloWorld");
	module.BindFunction<SetName>("SetName", "name");

	// Register custom component & system
	module.RegisterComponent<Audio::AudioEmitterComponent>();
	module.RegisterComponentSystem<Audio::AudioSystem>();

	// Register module hook on events
	module.OnUpdate.AddStatic([](float ts) 
	{
		auto& audioMgr = AudioManager::Get();
		audioMgr.SetGlobalVolume(Volume);

		audioMgr.AudioUpdate();
	});

	AudioManager::Get().Initialize();
}

void AudioModule_Shutdown()
{
}