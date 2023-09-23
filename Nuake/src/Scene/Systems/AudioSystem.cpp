#include "AudioSystem.h"

#include "Engine.h"
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
		AudioManager::Get().StopAll();

		return true;
	}

	void AudioSystem::Update(Timestep ts)
	{
		auto& audioManager = AudioManager::Get();

		// Update 3D listener of the audio system
		auto currentCamera = m_Scene->GetCurrentCamera();

		Vector3 direction = currentCamera->GetDirection();
		if (Engine::IsPlayMode())
		{
			direction *= Vector3(-1, -1, -1);
		}

		audioManager.SetListenerPosition(currentCamera->GetTranslation(), std::move(direction), currentCamera->GetUp());

		auto view = m_Scene->m_Registry.view<TransformComponent, AudioEmitterComponent>();
		for (auto& e : view)
		{
			auto [transformComponent, audioEmitterComponent] = view.get<TransformComponent, AudioEmitterComponent>(e);
			
			if (audioEmitterComponent.FilePath.empty())
			{
				// Doesn't have a file 
				continue;
			}

			const bool isPlaying = audioEmitterComponent.IsPlaying;
			const std::string absoluteFilePath = FileSystem::RelativeToAbsolute(audioEmitterComponent.FilePath);
			const bool isVoiceActive = audioManager.IsVoiceActive(absoluteFilePath);

			AudioRequest audioRequest;
			audioRequest.audioFile = absoluteFilePath;
			audioRequest.pan = audioEmitterComponent.Pan;
			audioRequest.volume = audioEmitterComponent.Volume;
			audioRequest.speed = audioEmitterComponent.PlaybackSpeed;
			audioRequest.spatialized = audioEmitterComponent.Spatialized;
			audioRequest.Loop = audioEmitterComponent.Loop;
			audioRequest.position = transformComponent.GetGlobalTransform()[3];
			audioRequest.MinDistance = audioEmitterComponent.MinDistance;
			audioRequest.MaxDistance = audioEmitterComponent.MaxDistance;
			audioRequest.AttenuationFactor = audioEmitterComponent.AttenuationFactor;

			if (isVoiceActive)
			{
				if (!isPlaying && audioEmitterComponent.Loop) 
				{
					audioManager.StopVoice(absoluteFilePath); // Stop audio
				}
				else
				{
					// Update the active voice with new params
					audioManager.UpdateVoice(audioRequest);
				}
			}

			if (isPlaying)
			{
				// Reset the play status to false since the audio has been fired
				if (!audioEmitterComponent.Loop)
				{
					audioManager.QueueWavAudio(std::move(audioRequest));
					audioEmitterComponent.IsPlaying = false;
				}
				else if (!isVoiceActive)
				{
					audioManager.QueueWavAudio(std::move(audioRequest));
				}
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
		AudioManager::Get().StopAll();
	}
}
