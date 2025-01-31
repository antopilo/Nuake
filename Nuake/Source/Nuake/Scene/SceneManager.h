#pragma once
#include "Nuake/Core/Core.h"
#include "Nuake/Resource/UUID.h"


namespace Nuake
{
	class File;
	class Scene;

	class SceneManager
	{
	public:
		static SceneManager& Get();

		SceneManager() = default;
		~SceneManager() = default;

		bool IsSceneLoaded(const UUID& uuid) const;
		Ref<Scene> GetScene(const UUID& uuid) const;

		Ref<Scene> SetCurrentScene(Ref<File> file);
		void UnloadScene(const UUID& uuid);

	private:
		std::map<UUID, Ref<Scene>> scenes;
	};
}