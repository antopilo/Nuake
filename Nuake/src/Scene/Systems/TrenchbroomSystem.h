#pragma once
#include <src/Scene/Systems/System.h>

namespace Nuake
{
	class TrenchbroomSystem : public System 
	{
	public:
		TrenchbroomSystem(Scene* scene);
		bool Init() override;

		void Draw() override {}

		void Update(Timestep ts) override;
		void FixedUpdate(Timestep ts) override;
		void EditorUpdate() override;

		void Exit() override;
	};
}
