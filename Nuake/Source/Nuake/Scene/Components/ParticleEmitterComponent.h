#pragma once

#include "Component.h"

#include "Nuake/Core/Core.h"
#include "Nuake/Core/Maths.h"
#include "Nuake/Resource/Serializable.h"

#include "Nuake/Scene/Systems/ParticleEmitter.h"


namespace Nuake
{
	class ParticleEmitterComponent : public Component
	{
		NUAKECOMPONENT(ParticleEmitterComponent, "Particle Emitter")

		static void InitializeComponentClass()
		{
			BindComponentField<&ParticleEmitterComponent::resFile>("Particle", "Particle");
			BindComponentField<&ParticleEmitterComponent::Amount>("Amount", "Amount");
				FieldFloatLimits(0.1f, 0.0f, 500.0f);
			BindComponentField<&ParticleEmitterComponent::LifeRandomness>("LifeRandomness", "Life Randomness");
				FieldFloatLimits(0.1f, 0.f, 9999.f);
			BindComponentField<&ParticleEmitterComponent::Life>("Life", "Life");
				FieldFloatLimits(0.1f, 0.0f, 100.0f);
			BindComponentField<&ParticleEmitterComponent::Rate>("Rate", "Rate");
				FieldFloatLimits(0.1f, 0.0f, 10.0f);
			BindComponentField<&ParticleEmitterComponent::ScaleRandomness>("ScaleRandomness", "Scale Randomness");
				FieldFloatLimits(0.01f, 0.0f, 0.f);
			BindComponentField<&ParticleEmitterComponent::ParticleScale>("ParticleScale", "Particle Scale");
			BindComponentField<&ParticleEmitterComponent::GlobalSpace>("GlobalSpace", "Global Space");
			BindComponentField<&ParticleEmitterComponent::Gravity>("Gravity", "Gravity");
			BindComponentField<&ParticleEmitterComponent::GravityRandom>("GravityRandom", "Gravity Random");
				FieldFloatLimits(0.01f, 0.0f, 1.0f);
			BindComponentField<&ParticleEmitterComponent::Radius>("Radius", "Radius");
				FieldFloatLimits(0.01f, 0.0f, 10.0f);
		}

	public:
		ParticleEmitterComponent() = default;
		~ParticleEmitterComponent() = default;

		ResourceFile resFile;
		Ref<Material> ParticleMaterial = CreateRef<Material>();

		float Amount;

		float LifeRandomness = 0.0f;
		float Life = 1.0f;
		float Rate = 0.0f;

		float ScaleRandomness = 0.0f;
		Vector3 ParticleScale = Vector3(0.1, 0.1, 0.1);

		bool GlobalSpace = false;

		Vector3 Gravity;
		float GravityRandom;

		// For now use a radius, later should use shape.
		float Radius;

		ParticleEmitter Emitter;

	public:
		json Serialize();
		bool Deserialize(const json& j);
	};
}