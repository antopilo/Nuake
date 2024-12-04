#include "VulkanShader.h"
#include "VulkanRenderer.h"

using namespace Nuake;

VulkanShader::VulkanShader(uint32_t* bytecode, uint32_t bytecodeLength, ShaderType type)
{
    Type = type;

    VkShaderModuleCreateInfo shaderModuleCI{};
    shaderModuleCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCI.codeSize = bytecodeLength;
    shaderModuleCI.pCode = bytecode;
    vkCreateShaderModule(VkRenderer::Get().GetDevice(), &shaderModuleCI, nullptr, &Module);
}