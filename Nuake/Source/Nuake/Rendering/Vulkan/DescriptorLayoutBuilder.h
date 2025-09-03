#pragma once

#include <volk/volk.h>

#include <vector>

namespace Nuake
{
	struct DescriptorLayoutBuilder
	{
		std::vector<VkDescriptorSetLayoutBinding> Bindings;

		void AddBinding(uint32_t binding, VkDescriptorType type, uint32_t count = 1);
		void Clear();
		VkDescriptorSetLayout Build(VkDevice device, VkShaderStageFlags shaderStages, void* pNext = nullptr, VkDescriptorSetLayoutCreateFlags flags = 0);
	};
}