#pragma once

#include <volk/volk.h>

#include <deque>
#include <span>
#include <vector>

namespace Nuake
{
	class DescriptorAllocatorGrowable
	{
	public:
		struct PoolSizeRatio 
		{
			VkDescriptorType Type;
			float Ratio;
		};

		void Init(VkDevice device, uint32_t initialSets, std::span<PoolSizeRatio> poolRatios);
		void ClearPools(VkDevice device);
		void DestroyPools(VkDevice device);

		VkDescriptorSet Allocate(VkDevice device, VkDescriptorSetLayout layout, void* pNext = nullptr);
	private:
		VkDescriptorPool GetPool(VkDevice device);
		VkDescriptorPool CreatePool(VkDevice device, uint32_t setCount, std::span<PoolSizeRatio> poolRatios);

		std::vector<PoolSizeRatio> Ratios;
		std::vector<VkDescriptorPool> FullPools;
		std::vector<VkDescriptorPool> ReadyPools;
		uint32_t SetsPerPool;
	};

	class DescriptorWriter {
		std::deque<VkDescriptorImageInfo> imageInfos;
		std::deque<VkDescriptorBufferInfo> bufferInfos;
		std::vector<VkWriteDescriptorSet> writes;

	public:
		void WriteImage(int binding, VkImageView image, VkSampler sampler, VkImageLayout layout, VkDescriptorType type);
		void WriteBuffer(int binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type);

		void Clear();
		void UpdateSet(VkDevice device, VkDescriptorSet set);
	};
}