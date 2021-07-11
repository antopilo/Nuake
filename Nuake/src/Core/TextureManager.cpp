#include "TextureManager.h"
#include "src/Rendering/Textures/Texture.h"

namespace Nuake
{
	std::map<std::string, Ref<Texture>> TextureManager::m_Registry;

	TextureManager* TextureManager::s_Instance = nullptr;

	bool TextureManager::IsTextureLoaded(const std::string path)
	{
		return m_Registry.find(path) != m_Registry.end();
	}

	Ref<Texture> TextureManager::GetTexture(const std::string path)
	{
		if (!IsTextureLoaded(path))
			m_Registry.emplace(path, new Texture(path));

		return m_Registry.at(path);
	}

	TextureManager* TextureManager::Get() { return s_Instance; }

	TextureManager::TextureManager()
	{
		s_Instance = this;
	}
}
