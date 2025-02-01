#include "AudioSceneSystem.h"

#include "Engine.h"
#include "Nuake/Scene/Scene.h"
#include "Nuake/Scene/Entities/Entity.h"
#include "Nuake/Scene/Components/AudioEmitterComponent.h"
#include "Nuake/FileSystem/File.h"
#include "Nuake/Audio/AudioManager.h"

#include <future>

namespace AudioModule
{
	using namespace Nuake;

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
		Vector3 position = currentCamera->GetTranslation();
		if (!Engine::IsPlayMode())
		{
			direction.x *= -1.0f;
			direction.z *= -1.0f;
		}

		audioManager.SetListenerPosition(position, std::move(direction), currentCamera->GetUp());

		auto view = m_Scene->m_Registry.view<TransformComponent, AudioEmitterComponent>();
		for (auto& e : view)
		{
			auto [transformComponent, audioEmitterComponent] = view.get<TransformComponent, AudioEmitterComponent>(e);

			if (audioEmitterComponent.FilePath.file == nullptr || !audioEmitterComponent.FilePath.file->Exist())
			{
				// Doesn't have a file 
				continue;
			}

			const bool isPlaying = audioEmitterComponent.IsPlaying;
			const std::string absoluteFilePath = audioEmitterComponent.FilePath.file->GetAbsolutePath();
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