#include "VulkanAllocatedBuffer.h"

#include "VulkanAllocator.h"
#include "VulkanCheck.h"
#include "VulkanRenderer.h"

#include "vk_mem_alloc.h"
using namespace Nuake;


AllocatedBuffer::AllocatedBuffer(size_t inSize, VkBufferUsageFlags inFlags, VmaMemoryUsage inUsage)
{
	// allocate buffer
	VkBufferCreateInfo bufferInfo = { .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.pNext = nullptr;
	bufferInfo.size = inSize;

	bufferInfo.usage = inFlags;

	VmaAllocationCreateInfo vmaallocInfo = {};
	vmaallocInfo.usage = inUsage;
	vmaallocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

	// allocate the buffer
	VK_CALL(vmaCreateBuffer(VulkanAllocator::Get().GetAllocator(), &bufferInfo, &vmaallocInfo, &Buffer, &Allocation,
		&Info));
}

AllocatedBuffer::~AllocatedBuffer()
{
	// TODO: deletion of buffer
}

GPUMeshBuffers::GPUMeshBuffers(std::vector<VkVertex> vertices, std::vector<uint32_t> indices)
{
	const size_t vertexBufferSize = vertices.size() * sizeof(VkVertex);
	const size_t indexBufferSize = indices.size() * sizeof(uint32_t);

	vertexBuffer = AllocatedBuffer(vertexBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
	VMA_MEMORY_USAGE_GPU_ONLY);

	//find the adress of the vertex buffer
	VkBufferDeviceAddressInfo deviceAddressInfo 
	{ 
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.buffer = vertexBuffer.GetBuffer()
	};

	vertexBufferAddress = vkGetBufferDeviceAddress(VkRenderer::Get().GetDevice(), &deviceAddressInfo);

	//create index buffer
	indexBuffer = AllocatedBuffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY);
}
