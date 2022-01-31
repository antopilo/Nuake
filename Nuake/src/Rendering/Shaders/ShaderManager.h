#pragma once
#include <map>
#include <string>

#include "src/Core/Core.h"
#include "Shader.h"

namespace Nuake
{

	class ShaderManager {
	private:
		static std::map<std::string, Scope<Shader>> m_Shaders;

	public:
		static Shader* GetShader(const std::string& path);
		static void RebuildShaders();
	};
}
