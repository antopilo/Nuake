#pragma once
#include "System.h"

namespace Nuake {
	class Scene;

	class ScriptingSystem : public System 
	{
	public:
		ScriptingSystem(Scene* scene);
		bool Init() override;
		void Update(Timestep ts) override;
		void Draw() override {}
		void FixedUpdate(Timestep ts) override;
		void Exit() override;
	};
}
