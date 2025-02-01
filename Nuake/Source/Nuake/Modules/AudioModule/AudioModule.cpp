#include "AudioModule.h"
#include "AudioSceneSystem.h"

#include "Nuake/Scene/SceneSystems.h"

void AudioModule_Startup()
{
	using namespace Nuake;

	SceneSystemDB::Get().RegisterSceneSystem<AudioModule::AudioSystem>();
}

void AudioModule_Shutdown()
{

}