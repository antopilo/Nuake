#pragma once

#include "Nuake/Core/Maths.h"
#include "Nuake/Resource/UUID.h"
#include "Nuake/Rendering/Vulkan/VkVertex.h"

#include <string>
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
		std::string Name;
		UUID ID;
		VkBuffer Buffer;
		VmaAllocation Allocation;
		VmaAllocationInfo Info;
		size_t Size;
		int LastUpdatedFrame = 0;

	public:
		AllocatedBuffer(size_t size, BufferUsage flags, MemoryUsage usage);
		AllocatedBuffer(const std::string& name, size_t size, BufferUsage flags, MemoryUsage usage);
		AllocatedBuffer() = default;
		~AllocatedBuffer();

		MemoryUsage MemUsage;
		BufferUsage BufUsage;

		VkBuffer GetBuffer() const { return Buffer; }
		VmaAllocation GetAllocation() const { return Allocation; }
		UUID GetID() const { return ID; }
		size_t GetSize() const { return Size; }
		int GetLastUpdated() const { return LastUpdatedFrame; }
		std::string GetName() const { return Name; }

		void Update();
	};

	// push constants for our mesh object draws
	struct GPUDrawPushConstants 
	{
		glm::mat4 worldMatrix;
		VkDeviceAddress vertexBuffer;
	};

}