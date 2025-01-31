#pragma once

#include <volk/volk.h>
#include "Nuake/Core/Maths.h"

#include <vector>

namespace Nuake
{
	class VulkanInit
	{
	public:
		VulkanInit() = delete;
		~VulkanInit() = delete;

		static VkCommandPoolCreateInfo CommandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);
		static VkCommandBufferAllocateInfo CommandBufferAllocateInfo(VkCommandPool pool, uint32_t count = 1);
		static VkFenceCreateInfo FenceCreateInfo(VkFenceCreateFlags flags = 0);
		static VkSemaphoreCreateInfo SemaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0);

		static VkCommandBufferBeginInfo CommandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0);
		static VkImageSubresourceRange  ImageSubResourceRange(VkImageAspectFlags  aspectMask);
		static VkSemaphoreSubmitInfo SemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);
		static VkCommandBufferSubmitInfo CommandBufferSubmitInfo(VkCommandBuffer cmd);
		static VkSubmitInfo2 SubmitInfo(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signalSemaphoreInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo);

		static VkImageCreateInfo ImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
		static VkImageViewCreateInfo ImageviewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);

		static VkRenderingAttachmentInfo AttachmentInfo(VkImageView view, VkClearValue* clear, VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		static VkRenderingAttachmentInfo DepthAttachmentInfo(VkImageView view, VkImageLayout layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
		static VkRenderingInfo RenderingInfo(Vector2 renderExtent,
										  std::vector<VkRenderingAttachmentInfo>& colorAttachments,
										  VkRenderingAttachmentInfo* depthAttachment
										  );

		static VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo();
		static VkPipelineShaderStageCreateInfo PipelineShaderStageCreateInfo(VkShaderStageFlagBits flags, VkShaderModule shader);
	};

	class VulkanUtil
	{
	public:
		VulkanUtil() = delete;
		~VulkanUtil() = delete;

		static void TransitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout, bool isDepth = false);
		static void CopyImageToImage(VkCommandBuffer cmd, VkImage source, VkImage destination, Vector2 srcSize, Vector2 dstSize);
	};
}
