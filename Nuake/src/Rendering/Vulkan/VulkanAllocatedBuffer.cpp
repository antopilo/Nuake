#include "VulkanAllocatedBuffer.h"

#include "VulkanAllocator.h"
#include "VulkanCheck.h"
#include "VulkanRenderer.h"

#include "vk_mem_alloc.h"
#include "VkResources.h"
using namespace Nuake;


AllocatedBuffer::AllocatedBuffer(size_t inSize, BufferUsage inFlags, MemoryUsage inUsage)
	: ID(UUID()),
	Size(inSize)
{
	// allocate buffer
	VkBufferCreateInfo bufferInfo = { .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.pNext = nullptr;
	bufferInfo.size = inSize;

	bufferInfo.usage = static_cast<VkBufferUsageFlags>(inFlags);

	VmaAllocationCreateInfo vmaallocInfo = {};
	vmaallocInfo.usage = static_cast<VmaMemoryUsage>(inUsage);
	vmaallocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

	// allocate the buffer
	VK_CALL(vmaCreateBuffer(VulkanAllocator::Get().GetAllocator(), &bufferInfo, &vmaallocInfo, &Buffer, &Allocation,
		&Info));
}

AllocatedBuffer::~AllocatedBuffer()
{
	// TODO: deletion of buffer
}
