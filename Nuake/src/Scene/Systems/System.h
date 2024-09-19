#pragma once

#include "src/Core/Timestep.h"
#include "src/Core/Core.h"
#include "src/Core/MulticastDelegate.h"

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

		MulticastDelegate<>& OnPreInit() { return preInitDelegate; }
		MulticastDelegate<>& OnPostInit() { return postInitDelegate; }

	protected:
		MulticastDelegate<> preInitDelegate;
		MulticastDelegate<> postInitDelegate;
	};
}
