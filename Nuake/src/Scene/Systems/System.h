#pragma once
#include "src/Core/Timestep.h"
#include "src/Core/Core.h"

namespace Nuake
{
	class Scene;
	class System {
	public:
		Scene* m_Scene;

		virtual bool Init() = 0;

		virtual void Draw() = 0;

		virtual void Update(Timestep ts) = 0;
		virtual void FixedUpdate(Timestep ts) = 0;
		virtual void EditorUpdate() {}
		virtual void Exit() = 0;
	};
}
