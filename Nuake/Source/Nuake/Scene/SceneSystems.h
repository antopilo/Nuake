#pragma once

#include "Nuake/Core/Core.h"

#include "Nuake/Scene/Systems/System.h"
#include "Nuake/Scene/Scene.h"

#include <set>
#include <functional>

namespace Nuake
{
	class Scene;
	class System;

	using SystemInitializerFunc = std::function<Ref<System>(Scene*)>;

	using SystemInitializerFunc = std::function<Ref<System>(Scene*)>;

	template<typename T>
	concept InheritsFromSystem = std::derived_from<T, System>;

	template<typename T>
	concept InheritsFromComponent = std::derived_from<T, Component>;

	class SceneSystemDB
	{
	private:
		std::vector<SystemInitializerFunc> Systems;
		
	public:
		static SceneSystemDB& Get()
		{
			static SceneSystemDB instance;
			return instance;
		};

		SceneSystemDB() = default;
		~SceneSystemDB() = default;

	public:
		template<InheritsFromSystem T>
		void RegisterSceneSystem()
		{
			Systems.push_back(&System::Instantiate<T>);
		}

		template<InheritsFromComponent T>
		void RegisterComponent()
		{
			T::InternalInitializeClass();
		}

		void InstantiateSystems(Scene* scene)
		{
			for (auto& system : Systems)
			{
				scene->m_Systems.push_back(system(scene));
			}
		}
	};
}