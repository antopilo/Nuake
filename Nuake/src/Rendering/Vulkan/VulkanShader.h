#pragma once

#include "src/Core/Core.h"

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

	public:
		VulkanShader(uint32_t* bytecode, uint32_t bytecodeLength, ShaderType type);
		~VulkanShader() = default;

		ShaderType GetType() const { return Type; }
		VkShaderModule GetModule() const { return Module; }
	};
}