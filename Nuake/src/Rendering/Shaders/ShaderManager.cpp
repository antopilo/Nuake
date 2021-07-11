#pragma once
#include "ShaderManager.h"

namespace Nuake
{
	std::map<std::string, Ref<Shader>> ShaderManager::m_Shaders = std::map<std::string, Ref<Shader>>();

	Ref<Shader> ShaderManager::GetShader(const std::string& path)
	{
		if (m_Shaders.find(path) == m_Shaders.end())
		{
			m_Shaders[path] = CreateRef<Shader>(path);
		}

		return m_Shaders[path];
	}
}
