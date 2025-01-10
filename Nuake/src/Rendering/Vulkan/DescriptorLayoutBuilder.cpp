#include "DescriptorLayoutBuilder.h"

#include "VulkanCheck.h"

using namespace Nuake;

void DescriptorLayoutBuilder::AddBinding(uint32_t binding, VkDescriptorType type, uint32_t count)
{
	VkDescriptorSetLayoutBinding newbind{};
	newbind.binding = binding;
	newbind.descriptorCount = count;
	newbind.descriptorType = type;
	Bindings.push_back(newbind);
}

void DescriptorLayoutBuilder::Clear()
{
	Bindings.clear();
}

VkDescriptorSetLayout DescriptorLayoutBuilder::Build(VkDevice device, VkShaderStageFlags shaderStages, void * pNext, VkDescriptorSetLayoutCreateFlags flags)
{
	for (auto& b : Bindings) {
		b.stageFlags |= shaderStages;
	}

	VkDescriptorSetLayoutCreateInfo info = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	info.pNext = pNext;

	info.pBindings = Bindings.data();
	info.bindingCount = (uint32_t)Bindings.size();
	info.flags = flags;
	

	VkDescriptorSetLayout set;
	VK_CALL(vkCreateDescriptorSetLayout(device, &info, nullptr, &set));

	return set;
}