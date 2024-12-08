#include "VulkanAllocatedBuffer.h"

#include "VulkanAllocator.h"
#include "VulkanCheck.h"
#include "VulkanRenderer.h"


using namespace Nuake;

#include "vk_mem_alloc.h"

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
