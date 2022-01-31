#pragma once
#include "ShaderManager.h"
#include "src/Core/Logger.h"

namespace Nuake
{
	std::map<std::string, Scope<Shader>> ShaderManager::m_Shaders = std::map<std::string, Scope<Shader>>();

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
				Logger::Log("Failed to rebuild shader: " + s.first, Nuake::CRITICAL);
			}
		}
	}
}
