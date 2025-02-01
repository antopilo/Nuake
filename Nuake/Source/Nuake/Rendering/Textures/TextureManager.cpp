#include "TextureManager.h"
#include "Nuake/Rendering/Textures/Texture.h"
#include "Nuake/Rendering/Vulkan/VulkanImage/VulkanImage.h"

#include "Nuake/Resource/StaticResources.h"
#include "Nuake/Rendering/Vulkan/VkResources.h"

#include <glad/glad.h>

namespace Nuake
{
	std::map<std::string, Ref<Texture>> TextureManager::m_Registry;
	std::map<std::string, Ref<VulkanImage>> TextureManager::m_Registry2;

	bool m_StaticTextureLoaded = false;
	TextureManager* TextureManager::s_Instance = nullptr;

	bool TextureManager::IsTextureLoaded(const std::string path)
	{
		return m_Registry.find(path) != m_Registry.end();
	}

	bool TextureManager::IsTextureLoaded2(const std::string path)
	{
		return m_Registry2.find(path) != m_Registry2.end();
	}

	void TextureManager::LoadStaticTextures()
	{
		using namespace StaticResources;
		m_Registry2.emplace(Resources_Images_nuake_logo_png_path, CreateRef<VulkanImage>(Resources_Images_nuake_logo_png, Resources_Images_nuake_logo_png_len));
		m_Registry2.emplace(Resources_Images_logo_white_png_path, CreateRef<VulkanImage>(Resources_Images_logo_white_png, Resources_Images_logo_white_png_len));
		m_Registry2.emplace(Resources_Gizmos_bone_png_path, CreateRef<VulkanImage>(Resources_Gizmos_bone_png, Resources_Gizmos_bone_png_len));
		m_Registry2.emplace(Resources_Gizmos_camera_png_path, CreateRef<VulkanImage>(Resources_Gizmos_camera_png, Resources_Gizmos_camera_png_len));
		m_Registry2.emplace(Resources_Gizmos_light_png_path, CreateRef<VulkanImage>(Resources_Gizmos_light_png, Resources_Gizmos_light_png_len));
		m_Registry2.emplace(Resources_Gizmos_light_directional_png_path, CreateRef<VulkanImage>(Resources_Gizmos_light_directional_png, Resources_Gizmos_light_directional_png_len));
		m_Registry2.emplace(Resources_Gizmos_light_spot_png_path, CreateRef<VulkanImage>(Resources_Gizmos_light_spot_png, Resources_Gizmos_light_spot_png_len));
		m_Registry2.emplace(Resources_Gizmos_player_png_path, CreateRef<VulkanImage>(Resources_Gizmos_player_png, Resources_Gizmos_player_png_len));
		m_Registry2.emplace(Resources_Gizmos_particles_png_path, CreateRef<VulkanImage>(Resources_Gizmos_particles_png, Resources_Gizmos_particles_png_len));
		m_Registry2.emplace(Resources_Gizmos_rigidbody_png_path, CreateRef<VulkanImage>(Resources_Gizmos_rigidbody_png, Resources_Gizmos_rigidbody_png_len));
		m_Registry2.emplace(Resources_Gizmos_sound_emitter_png_path, CreateRef<VulkanImage>(Resources_Gizmos_sound_emitter_png, Resources_Gizmos_sound_emitter_png_len));
		
		m_Registry2.emplace(Resources_Images_cube_png_path, CreateRef<VulkanImage>(Resources_Images_cube_png, Resources_Images_cube_png_len));
		m_Registry2.emplace(Resources_Images_logo_png_path, CreateRef<VulkanImage>(Resources_Images_logo_png, Resources_Images_logo_png_len));
		m_Registry2.emplace(Resources_Images_file_icon_png_path, CreateRef<VulkanImage>(Resources_Images_file_icon_png, Resources_Images_file_icon_png_len));
		m_Registry2.emplace(Resources_Images_env_file_icon_png_path, CreateRef<VulkanImage>(Resources_Images_env_file_icon_png, Resources_Images_env_file_icon_png_len));
		m_Registry2.emplace(Resources_Images_scene_icon_png_path, CreateRef<VulkanImage>(Resources_Images_scene_icon_png, Resources_Images_scene_icon_png_len));
		m_Registry2.emplace(Resources_Images_project_icon_png_path, CreateRef<VulkanImage>(Resources_Images_project_icon_png, Resources_Images_project_icon_png_len));
		m_Registry2.emplace(Resources_Images_csharp_icon_png_path, CreateRef<VulkanImage>(Resources_Images_csharp_icon_png, Resources_Images_csharp_icon_png_len));
		m_Registry2.emplace(Resources_Images_sln_icon_png_path, CreateRef<VulkanImage>(Resources_Images_sln_icon_png, Resources_Images_sln_icon_png_len));
		m_Registry2.emplace(Resources_Images_trenchbroom_icon_png_path, CreateRef<VulkanImage>(Resources_Images_trenchbroom_icon_png, Resources_Images_trenchbroom_icon_png_len));
		m_Registry2.emplace(Resources_Images_editor_icon_png_path, CreateRef<VulkanImage>(Resources_Images_editor_icon_png, Resources_Images_editor_icon_png_len));
		m_Registry2.emplace(Resources_Images_close_icon_png_path, CreateRef<VulkanImage>(Resources_Images_close_icon_png, Resources_Images_close_icon_png_len));
		m_Registry2.emplace(Resources_Images_maximize_icon_png_path, CreateRef<VulkanImage>(Resources_Images_maximize_icon_png, Resources_Images_maximize_icon_png_len));
		m_Registry2.emplace(Resources_Images_minimize_icon_png_path, CreateRef<VulkanImage>(Resources_Images_minimize_icon_png, Resources_Images_minimize_icon_png_len));
		m_Registry2.emplace(Resources_Images_restore_icon_png_path, CreateRef<VulkanImage>(Resources_Images_restore_icon_png, Resources_Images_restore_icon_png_len));
		m_Registry2.emplace(Resources_Images_folder_icon_png_path, CreateRef<VulkanImage>(Resources_Images_folder_icon_png, Resources_Images_folder_icon_png_len));
		m_Registry2.emplace(Resources_Images_audio_file_icon_png_path, CreateRef<VulkanImage>(Resources_Images_audio_file_icon_png, Resources_Images_audio_file_icon_png_len));
		m_Registry2.emplace("missing_texture", CreateRef<VulkanImage>(Resources_Images_missing_texture_png, Resources_Images_missing_texture_png_len));

		GPUResources::Get().AddTexture(m_Registry2["missing_texture"]);
		//
		//unsigned char whitePixel[] = { 255, 255, 255, 255 };
		//m_Registry.emplace("Resources/Textures/Default.png", CreateRef<Texture>(Vector2( 1, 1 ), GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, &whitePixel));
		//m_Registry.emplace("default", new Texture({ 1, 1 }, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, &whitePixel));
		//
		//unsigned char normalPixel[] = { 128, 128, 255, 255 };
		//m_Registry.emplace("Resources/Textures/DefaultNormal.png", CreateRef<Texture>(Vector2(1, 1), GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, &whitePixel));
		//
	}

	Ref<Texture> TextureManager::GetTexture(const std::string path)
	{
		if (!IsTextureLoaded(path))
			m_Registry.emplace(path, new Texture(path));

		return m_Registry.at(path);
	}

	Ref<VulkanImage> TextureManager::GetTexture2(const std::string path)
	{
		if (!IsTextureLoaded2(path))
		{
			m_Registry2.emplace(path, new VulkanImage(path));
			GPUResources::Get().AddTexture(m_Registry2[path]);
		}

		return m_Registry2.at(path);
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
