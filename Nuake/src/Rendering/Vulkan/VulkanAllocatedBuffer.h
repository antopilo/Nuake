#pragma once

#include "src/Core/Maths.h"

#include <volk/volk.h>
#include <vk_mem_alloc.h>


namespace Nuake
{
	class AllocatedBuffer
	{
	private:
		VkBuffer Buffer;
		VmaAllocation Allocation;
		VmaAllocationInfo Info;

	public:
		AllocatedBuffer(size_t size, VkBufferUsageFlags flags, VmaMemoryUsage usage);
		~AllocatedBuffer();
	};

	struct GPUMeshBuffers 
	{
		AllocatedBuffer indexBuffer;
		AllocatedBuffer vertexBuffer;
		VkDeviceAddress vertexBufferAddress;
	};

	// push constants for our mesh object draws
	struct GPUDrawPushConstants 
	{
		glm::mat4 worldMatrix;
		VkDeviceAddress vertexBuffer;
	};
}