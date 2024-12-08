#pragma once

#include "src/Core/Maths.h"
#include "src/Rendering/Vulkan/VkVertex.h"

#include <vector>

#include "volk/volk.h"

#include "vk_mem_alloc.h"

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
		AllocatedBuffer() = default;
		~AllocatedBuffer();

		VkBuffer GetBuffer() const { return Buffer; }
		VmaAllocation GetAllocation() const { return Allocation; }
	};


	// push constants for our mesh object draws
	struct GPUDrawPushConstants 
	{
		glm::mat4 worldMatrix;
		VkDeviceAddress vertexBuffer;
	};

	
	// holds the resources needed for a mesh
	class GPUMeshBuffers
	{
	public:
		AllocatedBuffer indexBuffer;
		AllocatedBuffer vertexBuffer;
		VkDeviceAddress vertexBufferAddress;

		GPUMeshBuffers(std::vector<VkVertex> vertices, std::vector<uint32_t> indices);
		GPUMeshBuffers() = default;
		~GPUMeshBuffers() = default;
	};
}