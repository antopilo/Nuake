#pragma once

#include "src/Core/Core.h"
#include "src/Resource/UUID.h"
#include "src/Rendering/Vertex.h"

#include "volk/volk.h"
#include "VulkanAllocatedBuffer.h"


namespace Nuake
{
	class VkMesh
	{
	private:
		UUID ID;
		
		Ref<AllocatedBuffer> IndexBuffer;
		Ref<AllocatedBuffer> VertexBuffer;

		VkDescriptorSet DescriptorSet;
		VkDescriptorSetLayout DescriptorLayout;

	public:
		VkMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
		~VkMesh() = default;

		Ref<AllocatedBuffer> GetVertexBuffer() const { return VertexBuffer; }
		Ref<AllocatedBuffer> GetIndexBuffer() const { return IndexBuffer; }

		UUID GetID() const { return ID; }

		VkDescriptorSet GetDescriptorSet()
		{
			return DescriptorSet;
		}

	private:
		void UploadToGPU(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
		void CreateDescriptorSet();
	};
}