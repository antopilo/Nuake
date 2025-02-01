#include "AudioModule.h"

#include "Nuake/Scene/SceneSystems.h"

#include "AudioSceneSystem.h"

void AudioModule_Startup()
{
	using namespace Nuake;

	SceneSystemDB::Get().RegisterSceneSystem<AudioModule::AudioSystem>();
}

void AudioModule_Shutdown()
{

}