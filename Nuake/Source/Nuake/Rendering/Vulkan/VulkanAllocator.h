#pragma once

#include "vk_mem_alloc.h"


namespace Nuake
{
	class VulkanAllocator
	{
	private:
		VmaAllocator Allocator;
		bool IsInitialized;

	public:
		static VulkanAllocator& Get()
		{
			static VulkanAllocator instance;
			return instance;
		};

		VulkanAllocator();
		~VulkanAllocator() = default;

		void Initialize(VkInstance inInstance, VkPhysicalDevice inGpu,
		VkDevice inDevice);

		VmaAllocator& GetAllocator()
		{
			return Allocator;
		}
	};
}