#include "VkMesh.h"
#include "VkResources.h"

#include "VulkanRenderer.h"
#include "VulkanAllocator.h"

using namespace Nuake;

VkMesh::VkMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) :
	DescriptorLayout(nullptr)
{
	// First we allocate the buffers on the GPU
	const size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
	const size_t indexBufferSize = indices.size() * sizeof(uint32_t);

	GPUResources& resources = GPUResources::Get();
	VertexBuffer = resources.CreateBuffer(vertexBufferSize, BufferUsage::STORAGE_BUFFER | BufferUsage::TRANSFER_DST, MemoryUsage::GPU_ONLY, "VertexBuffer");
	IndexBuffer = resources.CreateBuffer(indexBufferSize, BufferUsage::INDEX_BUFFER | BufferUsage::TRANSFER_DST, MemoryUsage::GPU_ONLY, "IndexBuffer");

	// Then we upload to data to those buffers.
	UploadToGPU(vertices, indices);
}

void VkMesh::UploadToGPU(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
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

	CreateDescriptorSet();
}

void VkMesh::CreateDescriptorSet()
{
	auto& vk = VkRenderer::Get();
	auto device = vk.GetDevice();

	if (!DescriptorLayout)
	{
		DescriptorLayoutBuilder builder;
		builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		DescriptorLayout = builder.Build(device, VK_SHADER_STAGE_ALL_GRAPHICS);
		DescriptorSet = vk.GetDescriptorAllocator().Allocate(device, DescriptorLayout);
	}

	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = GetVertexBuffer()->GetBuffer();
	bufferInfo.offset = 0;
	bufferInfo.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet bufferWrite = {};
	bufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	bufferWrite.pNext = nullptr;
	bufferWrite.dstBinding = 0;
	bufferWrite.dstSet = DescriptorSet;
	bufferWrite.descriptorCount = 1;
	bufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bufferWrite.pBufferInfo = &bufferInfo;
	vkUpdateDescriptorSets(device, 1, &bufferWrite, 0, nullptr);
}
