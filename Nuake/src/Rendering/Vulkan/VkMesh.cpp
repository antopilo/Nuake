#include "VkMesh.h"
#include "VkResources.h"

#include "VulkanRenderer.h"
#include "VulkanAllocator.h"

using namespace Nuake;

VkMesh::VkMesh(const std::vector<VkVertex>& vertices, const std::vector<uint32_t>& indices)
{
	// First we allocate the buffers on the GPU
	const size_t vertexBufferSize = vertices.size() * sizeof(VkVertex);
	const size_t indexBufferSize = indices.size() * sizeof(uint32_t);

	GPUResources& resources = GPUResources::Get();
	VertexBuffer = resources.CreateBuffer(vertexBufferSize, BufferUsage::STORAGE_BUFFER | BufferUsage::TRANSFER_DST, MemoryUsage::GPU_ONLY);
	IndexBuffer = resources.CreateBuffer(indexBufferSize, BufferUsage::INDEX_BUFFER | BufferUsage::TRANSFER_DST, MemoryUsage::GPU_ONLY);

	// Then we upload to data to those buffers.
	UploadtoGPU(vertices, indices);
}

void VkMesh::UploadtoGPU(const std::vector<VkVertex>& vertices, const std::vector<uint32_t>& indices)
{
	// Create a staging buffer
	AllocatedBuffer staging = AllocatedBuffer(VertexBuffer->GetSize() + IndexBuffer->GetSize(), BufferUsage::TRANSFER_SRC, MemoryUsage::CPU_ONLY);

	void* mappedData;
	vmaMapMemory(VulkanAllocator::Get().GetAllocator(), staging.GetAllocation(), &mappedData);

	// copy vertex buffer
	memcpy(mappedData, vertices.data(), VertexBuffer->GetSize());

	// copy index buffer
	memcpy((char*)mappedData + VertexBuffer->GetSize(), indices.data(), IndexBuffer->GetSize());

	VkRenderer::Get().ImmediateSubmit([&](VkCommandBuffer cmd) {
		VkBufferCopy vertexCopy{ 0 };
		vertexCopy.dstOffset = 0;
		vertexCopy.srcOffset = 0;
		vertexCopy.size = VertexBuffer->GetSize();

		vkCmdCopyBuffer(cmd, staging.GetBuffer(), VertexBuffer->GetBuffer(), 1, &vertexCopy);

		VkBufferCopy indexCopy{ 0 };
		indexCopy.dstOffset = 0;
		indexCopy.srcOffset = VertexBuffer->GetSize();
		indexCopy.size = IndexBuffer->GetSize();

		vkCmdCopyBuffer(cmd, staging.GetBuffer(), IndexBuffer->GetBuffer(), 1, &indexCopy);
	});
}