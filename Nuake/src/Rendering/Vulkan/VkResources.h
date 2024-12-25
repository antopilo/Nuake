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
		std::map<UUID, Ref<VkMesh>> Meshes;


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

		Ref<VkMesh> CreateMesh(const std::vector<VkVertex>& vertices, const std::vector<uint32_t>& indices)
		{
			Ref<VkMesh> mesh = CreateRef<VkMesh>(vertices, indices);
			Meshes[mesh->GetID()] = mesh;
			return mesh;
		}

		bool AddMesh(const Ref<VkMesh>& mesh)
		{
			const UUID id = mesh->GetID();
			if (Meshes.find(id) == Meshes.end())
			{
				Meshes[id] = mesh;
				return true;
			}
			Logger::Log("Mesh with ID already exists", "vulkan", CRITICAL);
			return false;
		}

		Ref<VkMesh> GetMesh(const UUID& id)
		{
			if (Meshes.find(id) != Meshes.end())
			{
				return Meshes[id];
			}
			Logger::Log("Mesh with ID does not exist", "vulkan", CRITICAL);
			return nullptr;
		}
	};
}