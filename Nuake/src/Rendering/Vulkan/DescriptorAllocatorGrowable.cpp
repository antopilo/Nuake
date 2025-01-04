#include "DescriptorAllocatorGrowable.h"
#include "VulkanCheck.h"

using namespace Nuake;

VkDescriptorPool DescriptorAllocatorGrowable::GetPool(VkDevice device)
{
	VkDescriptorPool newPool;
	if (ReadyPools.size() != 0) {
		newPool = ReadyPools.back();
		ReadyPools.pop_back();
	}
	else {
		//need to create a new pool
		newPool = CreatePool(device, SetsPerPool, Ratios);

		SetsPerPool = SetsPerPool * 1.5;
		if (SetsPerPool > 4092) {
			SetsPerPool = 4092;
		}
	}

	return newPool;
}

VkDescriptorPool DescriptorAllocatorGrowable::CreatePool(VkDevice device, uint32_t setCount, std::span<PoolSizeRatio> poolRatios)
{
	std::vector<VkDescriptorPoolSize> poolSizes;
	for (PoolSizeRatio ratio : poolRatios) {
		poolSizes.push_back(VkDescriptorPoolSize{
			.type = ratio.Type,
			.descriptorCount = uint32_t(ratio.Ratio * setCount)
		});
	}

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = 0;
	pool_info.maxSets = setCount;
	pool_info.poolSizeCount = (uint32_t)poolSizes.size();
	pool_info.pPoolSizes = poolSizes.data();

	VkDescriptorPool newPool;
	vkCreateDescriptorPool(device, &pool_info, nullptr, &newPool);
	return newPool;
}

void DescriptorAllocatorGrowable::Init(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios)
{
	Ratios.clear();

	for (auto r : poolRatios) {
		Ratios.push_back(r);
	}

	VkDescriptorPool newPool = CreatePool(device, maxSets, poolRatios);

	SetsPerPool = maxSets * 1.5; //grow it next allocation

	ReadyPools.push_back(newPool);
}

void DescriptorAllocatorGrowable::ClearPools(VkDevice device)
{
	for (auto p : ReadyPools) {
		vkResetDescriptorPool(device, p, 0);
	}
	for (auto p : FullPools) {
		vkResetDescriptorPool(device, p, 0);
		ReadyPools.push_back(p);
	}
	FullPools.clear();
}

void DescriptorAllocatorGrowable::DestroyPools(VkDevice device)
{
	for (auto p : ReadyPools) {
		vkDestroyDescriptorPool(device, p, nullptr);
	}
	ReadyPools.clear();
	for (auto p : FullPools) {
		vkDestroyDescriptorPool(device, p, nullptr);
	}
	FullPools.clear();
}

VkDescriptorSet DescriptorAllocatorGrowable::Allocate(VkDevice device, VkDescriptorSetLayout layout, void* pNext)
{
	//get or create a pool to allocate from
	VkDescriptorPool poolToUse = GetPool(device);

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.pNext = pNext;
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = poolToUse;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &layout;

	VkDescriptorSet ds;
	VkResult result = vkAllocateDescriptorSets(device, &allocInfo, &ds);

	//allocation failed. Try again
	if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL) {

		FullPools.push_back(poolToUse);

		poolToUse = GetPool(device);
		allocInfo.descriptorPool = poolToUse;

		VK_CALL(vkAllocateDescriptorSets(device, &allocInfo, &ds));
	}

	ReadyPools.push_back(poolToUse);
	return ds;
}

void Nuake::DescriptorWriter::WriteImage(int binding, VkImageView image, VkSampler sampler, VkImageLayout layout, VkDescriptorType type)
{
	VkDescriptorImageInfo& info = imageInfos.emplace_back(VkDescriptorImageInfo{
		.sampler = sampler,
		.imageView = image,
		.imageLayout = layout
	});

	VkWriteDescriptorSet write = { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };

	write.dstBinding = binding;
	write.dstSet = VK_NULL_HANDLE; //left empty for now until we need to write it
	write.descriptorCount = 1;
	write.descriptorType = type;
	write.pImageInfo = &info;

	writes.push_back(write);
}

void Nuake::DescriptorWriter::WriteBuffer(int binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type)
{
}

void Nuake::DescriptorWriter::Clear()
{
	imageInfos.clear();
	writes.clear();
	bufferInfos.clear();
}

void Nuake::DescriptorWriter::UpdateSet(VkDevice device, VkDescriptorSet set)
{
	for (VkWriteDescriptorSet& write : writes) {
		write.dstSet = set;
	}

	vkUpdateDescriptorSets(device, (uint32_t)writes.size(), writes.data(), 0, nullptr);
}

