#include "ShaderManager.h"

using namespace Nuake;

void VkShaderManager::AddShader(const std::string& shaderName, Ref<VulkanShader> shader)
{
	Shaders[shaderName] = shader;
}

Ref<VulkanShader> VkShaderManager::GetShader(const std::string& shaderName)
{
	if (Shaders.find(shaderName) != Shaders.end())
	{
		return Shaders[shaderName];
	}

	assert(false && "Shader not found by name");
}