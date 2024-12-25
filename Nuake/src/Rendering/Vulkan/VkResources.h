#pragma once
#include "src/Core/Core.h"
#include "src/Resource/UUID.h"

#include "VulkanAllocatedBuffer.h"
#include <src/Core/Logger.h>

namespace Nuake
{
	class GPUResources
	{
	private:
		std::map<UUID, Ref<AllocatedBuffer>> Buffers;

	public:
		static GPUResources& Get()
		{
			static GPUResources instance;
			return instance;
		};

		GPUResources() = default;
		~GPUResources() = default;

		Ref<AllocatedBuffer> CreateBuffer(size_t size, BufferUsage flags, MemoryUsage usage)
		{
			Ref<AllocatedBuffer> buffer = CreateRef<AllocatedBuffer>(size, flags, usage);
			Buffers[buffer->GetID()] = buffer;
			return buffer;
		}

		bool AddBuffer(const Ref<AllocatedBuffer>& buffer)
		{
			const UUID id = buffer->GetID();
			if (Buffers.find(id) == Buffers.end())
			{
				Buffers[id] = buffer;
				return true;
			}

			Logger::Log("Buffer with ID already exists", "vulkan", CRITICAL);
			return false;
		}

		Ref<AllocatedBuffer> GetBuffer(const UUID& id)
		{
			if (Buffers.find(id) != Buffers.end())
			{
				return Buffers[id];
			}

			Logger::Log("Buffer with ID does not exist", "vulkan", CRITICAL);
			return nullptr;
		}
	};
}