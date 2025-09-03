#pragma once

#include "Nuake/Core/Timestep.h"
#include "Nuake/Core/Core.h"
#include "Nuake/Core/MulticastDelegate.h"

#include <concepts>

namespace Nuake
{
	class System;

	template<typename T>
	concept IsSceneSystem = std::derived_from<T, System>;

	class Scene;
	class System {
	public:
		Scene* m_Scene;

		template<IsSceneSystem T>
		static Ref<System> Instantiate(Scene* scene)
		{
			return std::static_pointer_cast<System>(CreateRef<T>(scene));
		}

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
