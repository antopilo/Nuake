#pragma once
#include "src/Core/Core.h"
#include "src/Core/Logger.h"
#include "src/Resource/UUID.h"

#include "src/Rendering/Vertex.h"
#include "src/Rendering/Vulkan/VulkanAllocatedBuffer.h"
#include "src/Rendering/Vulkan/VulkanImage/VulkanImage.h"
#include "src/Rendering/Vulkan/VkMesh.h"
#include "src/Rendering/Textures/TextureManager.h"

namespace Nuake
{
	class GPUResources
	{
	private:
		std::map<UUID, Ref<AllocatedBuffer>> Buffers;
		std::map<UUID, Ref<VkMesh>> Meshes;
		std::map<UUID, Ref<VulkanImage>> Images;

	public:
		static GPUResources& Get()
		{
			static GPUResources instance;
			return instance;
		};

		GPUResources() = default;
		~GPUResources() = default;

		Ref<AllocatedBuffer> CreateBuffer(size_t size, BufferUsage flags, MemoryUsage usage, const std::string& name = "")
		{
			Ref<AllocatedBuffer> buffer = CreateRef<AllocatedBuffer>(name, size, flags, usage);
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

		std::vector<Ref<AllocatedBuffer>> GetAllBuffers()
		{
			std::vector<Ref<AllocatedBuffer>> allBuffers;
			allBuffers.reserve(Buffers.size());
			for (const auto& [id, buffer] : Buffers)
			{
				allBuffers.push_back(buffer);
			}
			return allBuffers;
		}

		Ref<VkMesh> CreateMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
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

		bool AddTexture(Ref<VulkanImage> image)
		{
			const UUID id = image->GetID();
			if (Images.find(id) == Images.end())
			{
				Images[id] = image;
				return true;
			}

			Logger::Log("Buffer with ID already exists", "vulkan", CRITICAL);
			return false;
		}

        Ref<VulkanImage> GetTexture(const UUID& id)
        {
            if (Images.find(id) != Images.end())
            {
                return Images[id];
            }

            Logger::Log("Mesh with ID does not exist", "vulkan", CRITICAL);
			return TextureManager::Get()->GetTexture2("missing_texture");
        }
	};
}