#include "SceneManager.h"
#include <src/Resource/ResourceManager.h>

using namespace Nuake;

SceneManager& SceneManager::Get()
{
	static SceneManager instance;
	return instance;
}

bool SceneManager::IsSceneLoaded(const UUID& uuid) const
{
	return scenes.find(uuid) != scenes.end();
}

Ref<Scene> SceneManager::GetScene(const UUID& uuid) const
{
	if (!IsSceneLoaded(uuid))
	{
		return nullptr;
	}

	return Ref<Scene>();
}

Ref<Scene> SceneManager::LoadScene(Ref<File> file)
{

	return Ref<Scene>();
}



