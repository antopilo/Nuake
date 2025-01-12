#pragma once
#include "src/Core/Core.h"
#include "src/Rendering/Vulkan/VulkanShader.h"

namespace Nuake
{
	class VkShaderManager
	{
	private:
		std::map<std::string, Ref<VulkanShader>> Shaders;

	public:
		static VkShaderManager& Get()
		{
			static VkShaderManager instance;
			return instance;
		};

		void AddShader(const std::string& shaderName, Ref<VulkanShader> shader);
		Ref<VulkanShader> GetShader(const std::string& shaderName);
	};
}