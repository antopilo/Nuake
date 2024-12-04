#pragma once

#include "src/Core/Core.h"

#include <string>

namespace Nuake
{
	class VulkanShader;

	class ShaderCompiler
	{
	public:

		static ShaderCompiler& Get()
		{
			static ShaderCompiler instance;
			return instance;
		}

		Ref<VulkanShader> CompileShader(const std::string& path);
	};
}