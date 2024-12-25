#pragma once

#include "src/Core/Maths.h"
#include "src/Rendering/Vulkan/VkVertex.h"

#include <vector>

#include "volk/volk.h"

#include "vk_mem_alloc.h"

namespace Nuake
{
	enum BufferUsage
	{
		TRANSFER_SRC = 0x00000001,
		TRANSFER_DST = 0x00000002,
		INDEX_BUFFER = 0x00000040,
		STORAGE_BUFFER = 0x00000020,
	};

	inline BufferUsage operator|(BufferUsage a, BufferUsage b)
	{
		return static_cast<BufferUsage>(static_cast<int>(a) | static_cast<int>(b));
	}

	enum MemoryUsage
	{
		GPU_ONLY = 1,
		CPU_ONLY = 2,
		CPU_TO_GPU = 3,
		GPU_TO_CPU = 4,
	};

	class AllocatedBuffer
	{
	private:
		VkBuffer Buffer;
		VmaAllocation Allocation;
		VmaAllocationInfo Info;

	public:
		AllocatedBuffer(size_t size, BufferUsage flags, MemoryUsage usage);
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