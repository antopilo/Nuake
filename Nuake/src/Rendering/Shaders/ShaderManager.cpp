#include "ShaderManager.h"
#include "src/Core/Logger.h"

#include "src/Resource/StaticResources.h"

#define LoadEmbeddedShader(file) \
m_Shaders[file##_path] = CreateScope<Shader>(file##_path, std::string(reinterpret_cast<const char*>(file), reinterpret_cast<const char*>(file) + file##_len));

namespace Nuake
{
	std::map<std::string, Scope<Shader>> ShaderManager::m_Shaders = std::map<std::string, Scope<Shader>>();

	void ShaderManager::LoadShaders()
	{
		using namespace StaticResources;
		LoadEmbeddedShader(Resources_Shaders_outline_shader);
		LoadEmbeddedShader(Resources_Shaders_atmospheric_sky_shader);
		LoadEmbeddedShader(Resources_Shaders_barrel_distortion_shader);
		LoadEmbeddedShader(Resources_Shaders_bloom_shader);
		LoadEmbeddedShader(Resources_Shaders_blur_shader);	
		LoadEmbeddedShader(Resources_Shaders_BRD_shader);
		LoadEmbeddedShader(Resources_Shaders_combine_shader);
		LoadEmbeddedShader(Resources_Shaders_copy_shader);
		LoadEmbeddedShader(Resources_Shaders_deferred_shader);
		LoadEmbeddedShader(Resources_Shaders_depth_aware_blur_shader);
		LoadEmbeddedShader(Resources_Shaders_dither_shader);
		LoadEmbeddedShader(Resources_Shaders_dof_shader);
		LoadEmbeddedShader(Resources_Shaders_flat_shader);
		LoadEmbeddedShader(Resources_Shaders_gbuffer_shader);
		LoadEmbeddedShader(Resources_Shaders_gbuffer_skinned_shader);
		LoadEmbeddedShader(Resources_Shaders_gizmo_shader);
		LoadEmbeddedShader(Resources_Shaders_line_shader);
		//LoadEmbeddedShader(Resources_Shaders_pbr_shader);
		LoadEmbeddedShader(Resources_Shaders_sdf_text_shader);
		LoadEmbeddedShader(Resources_Shaders_shadowMap_shader);
		LoadEmbeddedShader(Resources_Shaders_shadowMap_skinned_shader);
		LoadEmbeddedShader(Resources_Shaders_skybox_shader);
		LoadEmbeddedShader(Resources_Shaders_ssao_shader);
		LoadEmbeddedShader(Resources_Shaders_ssr_shader);
		LoadEmbeddedShader(Resources_Shaders_tonemap_shader);
		LoadEmbeddedShader(Resources_Shaders_ui_shader);
		LoadEmbeddedShader(Resources_Shaders_vignette_shader);
		LoadEmbeddedShader(Resources_Shaders_volumetric_shader);
	}

	Shader* ShaderManager::GetShader(const std::string& path)
	{
		if (m_Shaders.find(path) == m_Shaders.end())
		{
			m_Shaders[path] = CreateScope<Shader>(path);
		}

		return m_Shaders[path].get();
	}

	void ShaderManager::RebuildShaders()
	{
		for (auto& s : m_Shaders)
		{
			if (!s.second->Rebuild())
			{
				Logger::Log("Failed to rebuild shader: " + s.first, "shader", Nuake::CRITICAL);
			}
		}
	}
}
