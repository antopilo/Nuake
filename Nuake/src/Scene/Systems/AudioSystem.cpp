#include "AudioSystem.h"

#include "src/Scene/Scene.h"
#include "src/Scene/Entities/Entity.h"
#include "src/Scene/Components/AudioEmitterComponent.h"

#include "src/Audio/AudioManager.h"

#include <future>


namespace Nuake
{
	AudioSystem::AudioSystem(Scene* scene)
	{
		m_Scene = scene;
	}

	bool AudioSystem::Init()
	{
		return true;
	}

	void AudioSystem::Update(Timestep ts)
	{
		auto view = m_Scene->m_Registry.view<TransformComponent, AudioEmitterComponent>();
		for (auto& e : view)
		{
			auto [transformComponent, audioEmitterComponent] = view.get<TransformComponent, AudioEmitterComponent>(e);
			
			if (audioEmitterComponent.FilePath.empty())
			{
				continue;
			}

			if (audioEmitterComponent.IsPlaying)
			{
				const std::string absoluteFilePath = FileSystem::RelativeToAbsolute(audioEmitterComponent.FilePath);

				AudioRequest audioRequest;
				audioRequest.audioFile = absoluteFilePath;
				audioRequest.pan = audioEmitterComponent.Pan;
				audioRequest.volume = audioEmitterComponent.Volume;
				audioRequest.speed = audioEmitterComponent.PlaybackSpeed;

				AudioManager::Get().QueueWavAudio(std::move(audioRequest));
				audioEmitterComponent.IsPlaying = false;
			}
		}
	}

	void AudioSystem::FixedUpdate(Timestep ts)
	{

	}

	void AudioSystem::EditorUpdate()
	{

	}

	void AudioSystem::Exit()
	{

	}
}
