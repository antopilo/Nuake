#include "TextureManager.h"
#include "src/Rendering/Textures/Texture.h"

#include "src/Resource/StaticResources.h"
#include <glad/glad.h>

namespace Nuake
{
	std::map<std::string, Ref<Texture>> TextureManager::m_Registry;

	bool m_StaticTextureLoaded = false;
	TextureManager* TextureManager::s_Instance = nullptr;

	bool TextureManager::IsTextureLoaded(const std::string path)
	{
		return m_Registry.find(path) != m_Registry.end();
	}

	void TextureManager::LoadStaticTextures()
	{
		using namespace StaticResources;
		m_Registry.emplace(Resources_Gizmos_bone_png_path, new Texture(Resources_Gizmos_bone_png, Resources_Gizmos_bone_png_len));
		m_Registry.emplace(Resources_Gizmos_camera_png_path, new Texture(Resources_Gizmos_camera_png, Resources_Gizmos_camera_png_len));
		m_Registry.emplace(Resources_Gizmos_light_png_path, new Texture(Resources_Gizmos_light_png, Resources_Gizmos_light_png_len));
		m_Registry.emplace(Resources_Gizmos_player_png_path, new Texture(Resources_Gizmos_player_png, Resources_Gizmos_player_png_len));

		m_Registry.emplace(Resources_Images_nuake_logo_png_path, new Texture(Resources_Images_nuake_logo_png, Resources_Images_nuake_logo_png_len));
		m_Registry.emplace(Resources_Images_logo_white_png_path, new Texture(Resources_Images_logo_white_png, Resources_Images_logo_white_png_len));
		m_Registry.emplace(Resources_Images_logo_png_path, new Texture(Resources_Images_logo_png, Resources_Images_logo_png_len));

		unsigned char whitePixel[] = { 255, 255, 255, 255 };
		m_Registry.emplace("Resources/Textures/Default.png", new Texture({1, 1}, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, &whitePixel));
		m_Registry.emplace("default", new Texture({ 1, 1 }, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, &whitePixel));

		unsigned char normalPixel[] = { 128, 128, 255, 255 };
		m_Registry.emplace("Resources/Textures/DefaultNormal.png", new Texture({ 1, 1 }, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, &whitePixel));

		// Icons
		m_Registry.emplace(Resources_Images_folder_icon_png_path, new Texture(Resources_Images_folder_icon_png, Resources_Images_folder_icon_png_len));
	}

	Ref<Texture> TextureManager::GetTexture(const std::string path)
	{
		if (!IsTextureLoaded(path))
			m_Registry.emplace(path, new Texture(path));

		return m_Registry.at(path);
	}

	TextureManager* TextureManager::Get() 
	{ 
		if (!m_StaticTextureLoaded)
		{
			s_Instance->LoadStaticTextures();
			m_StaticTextureLoaded = true;
		}

		return s_Instance; 
	}

	TextureManager::TextureManager()
	{
		s_Instance = this;
		LoadStaticTextures();
	}
}
