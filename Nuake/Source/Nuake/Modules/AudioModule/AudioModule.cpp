#include "AudioModule.h"
#include "AudioSceneSystem.h"
#include "Nuake/Core/Maths.h"
#include "Nuake/Scene/SceneSystems.h"
#include "Nuake/Modules/ModuleDB.h"
#include <Nuake/Core/Logger.h>

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
	module.Description = "Core audio module.";

	module.RegisterSetting<&Volume>("Volume");
	module.RegisterSetting<&Muted>("Muted");

	module.BindFunction<PlayAudio>("PlayAudio", "id", "volume");
	module.BindFunction<StopAudio>("StopAudio");

	Logger::Log("AudioModule exposed API:", "Module", VERBOSE);

	auto reflection = module.Resolve();
	auto metaTypeid = entt::hashed_string(module.Name.c_str());
	auto s = reflection.func(metaTypeid);
	for (auto [id, func] : reflection.func())
	{
		const std::string_view returnType = func.ret().info().name();
		const std::string_view funcName = module.GetTypeName(id);

		std::string msg = std::string(returnType) + " " + std::string(funcName) + "(";
		auto argNames = module.GetFuncArgNames(id);
		std::vector<std::string_view> args;
		for (int i = 0; i < func.arity(); i++)
		{
			const std::string argType = std::string(func.arg(i).info().name());
			args.push_back(argType);

			msg += argType + " " + argNames[i];

			if (i < func.arity() - 1)
			{
				msg += ", ";
			}
		}
		msg += ")";

		Logger::Log(msg, "", VERBOSE);
	}

	SceneSystemDB::Get().RegisterSceneSystem<Audio::AudioSystem>();
}

void AudioModule_Shutdown()
{

}