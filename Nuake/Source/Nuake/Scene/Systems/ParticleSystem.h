#pragma once
#include "Nuake/Scene/Systems/System.h"
#include "Nuake/Core/Maths.h"

#include <random>


namespace Nuake
{
	class ParticleSystem : public System
	{
	public:
		ParticleSystem(Scene* scene);
		bool Init() override;
		void Update(Timestep ts) override;
		void Draw() override {}
		void EditorUpdate() override;
		void FixedUpdate(Timestep ts) override;
		void Exit() override;
	};
}
