#pragma once

#include "src/Core/Core.h"
#include "src/Resource/UUID.h"

#include "VulkanAllocatedBuffer.h"


namespace Nuake
{
	class VkMesh
	{
	private:
		UUID ID;
		
		Ref<AllocatedBuffer> IndexBuffer;
		Ref<AllocatedBuffer> VertexBuffer;

	public:
		VkMesh(const std::vector<VkVertex>& vertices, const std::vector<uint32_t>& indices);
		~VkMesh() = default;

		Ref<AllocatedBuffer> GetVertexBuffer() const { return VertexBuffer; }
		Ref<AllocatedBuffer> GetIndexBuffer() const { return IndexBuffer; }

		UUID GetID() const { return ID; }

	private:
		void UploadtoGPU(const std::vector<VkVertex>& vertices, const std::vector<uint32_t>& indices);
	};
}