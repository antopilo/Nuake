#pragma once

#include "Component.h"
#include "FieldTypes.h"

#include "src/Core/Core.h"
#include "src/Resource/Resource.h"
#include "src/Resource/Serializable.h"

namespace Nuake {

	class AudioEmitterComponent : public Component
	{
		NUAKECOMPONENT(AudioEmitterComponent, "Audio Emitter")

		static void InitializeComponentClass()
		{
			BindComponentField<&AudioEmitterComponent::FilePath>("FilePath", "File Path");
				ResourceFileRestriction("_AudioFile");
			
			BindComponentField<&AudioEmitterComponent::IsPlaying>("IsPlaying", "Is Playing");
			BindComponentField<&AudioEmitterComponent::Loop>("Loop", "Loop");

			BindComponentField<&AudioEmitterComponent::Volume>("Volume", "Volume");
				FieldFloatLimits(0.001f, 0.0f, 2.0f);
			BindComponentField<&AudioEmitterComponent::Pan>("Pan", "Pan");
				FieldFloatLimits(0.01f, -1.0f, 1.0f);
			BindComponentField<&AudioEmitterComponent::PlaybackSpeed>("PlaybackSpeed", "Playback Speed");
				FieldFloatLimits(0.01f, 0.0001f, 0.f);
			
			BindComponentField<&AudioEmitterComponent::Spatialized>("Spatialized", "Spatialized");
			
			BindComponentField<&AudioEmitterComponent::MinDistance>("MinDistance", "Min Distance");
				FieldFloatLimits(0.001f, 0.f, 0.f);
			BindComponentField<&AudioEmitterComponent::MaxDistance>("MaxDistance", "Max Distance");
				FieldFloatLimits(0.001f, 0.f, 0.f);
			BindComponentField<&AudioEmitterComponent::AttenuationFactor>("AttenuationFactor", "Attenuation Factor");
				FieldFloatLimits(0.001f, 0.f, 0.f);
		}

	public:
		ResourceFile FilePath;

		bool IsPlaying = false;
		bool Loop = false;

		float Volume = 1.0f;
		float Pan = 0.0f;
		float PlaybackSpeed = 1.0f;
		
		bool Spatialized = false;
		float MinDistance = 1.0f;
		float MaxDistance = 10.0f;
		float AttenuationFactor = 1.0f;

		json Serialize();
		bool Deserialize(const json& j);
	};
}
