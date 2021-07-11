#pragma once
#include <src/Scene/Systems/System.h>

namespace Nuake
{
	class TrenchbroomSystem : public System 
	{
	public:
		TrenchbroomSystem(Scene* scene);
		bool Init() override;
		void Update(Timestep ts) override;
		void Draw() override {}
		void FixedUpdate(Timestep ts) override;
		void Exit() override;
	};
}
