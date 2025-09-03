#pragma once

#include "Nuake/Core/Core.h"
#include <volk/volk.h>

namespace Nuake
{
	enum class ShaderType
	{
		Vertex,
		Fragment,
		Compute
	};

	class VulkanShader
	{
	private:
		VkShaderModule Module;
		ShaderType Type;
		std::string SourcePath;

	public:
		VulkanShader(uint32_t* bytecode, uint32_t bytecodeLength, ShaderType type);
		~VulkanShader() = default;

		ShaderType GetType() const { return Type; }
		VkShaderModule GetModule() const { return Module; }

		// For shader rebuilding only
		void SetSourcePath(const std::string& path) { SourcePath = path; }
		std::string GetSourcePath() const { return SourcePath; }
	};
}