#pragma once
#include <src/Scene/Systems/System.h>

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

	private:
	};
}
