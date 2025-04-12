#include "VkShaderManager.h"
#include "ShaderCompiler.h"

#include "SceneRenderPipeline.h"

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

void VkShaderManager::RecompileShaders()
{
	ShaderCompiler compiler;

	for (auto& shader : Shaders)
	{
		const auto& filePath = shader.second->GetSourcePath();
		shader.second = compiler.CompileShader(filePath);
	}

	VkRenderer::Get().RecreatePipelines();
}