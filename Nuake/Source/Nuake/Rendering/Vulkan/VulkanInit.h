#pragma once

#include <volk/volk.h>
#include "Nuake/Core/Maths.h"
#include "Nuake/Rendering/Vulkan/VulkanRenderer.h"

#include <string>
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

		template<typename VulkanHandleType>
		static constexpr VkObjectType GetVkObjectType()
		{
			if constexpr (std::is_same_v<VulkanHandleType, VkBuffer>) {
				return VK_OBJECT_TYPE_BUFFER;
			}
			else if constexpr (std::is_same_v<VulkanHandleType, VkImage>) {
				return VK_OBJECT_TYPE_IMAGE;
			}
			else if constexpr (std::is_same_v<VulkanHandleType, VkShaderModule>) {
				return VK_OBJECT_TYPE_SHADER_MODULE;
			}
			else if constexpr (std::is_same_v<VulkanHandleType, VkPipeline>) {
				return VK_OBJECT_TYPE_PIPELINE;
			}
			else if constexpr (std::is_same_v<VulkanHandleType, VkDescriptorSet>) {
				return VK_OBJECT_TYPE_DESCRIPTOR_SET;
			}
			else if constexpr (std::is_same_v<VulkanHandleType, VkPipelineLayout>) {
				return VK_OBJECT_TYPE_PIPELINE_LAYOUT;
			}
			else if constexpr (std::is_same_v<VulkanHandleType, VkRenderPass>) {
				return VK_OBJECT_TYPE_RENDER_PASS;
			}
			else if constexpr (std::is_same_v<VulkanHandleType, VkFramebuffer>) {
				return VK_OBJECT_TYPE_FRAMEBUFFER;
			}
			else if constexpr (std::is_same_v<VulkanHandleType, VkDescriptorSetLayout>) {
				return VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT;
			}
			else if constexpr (std::is_same_v<VulkanHandleType, VkBufferView>) {
				return VK_OBJECT_TYPE_BUFFER_VIEW;
			}
			else if constexpr (std::is_same_v<VulkanHandleType, VkImageView>) {
				return VK_OBJECT_TYPE_IMAGE_VIEW;
			}
			else if constexpr (std::is_same_v<VulkanHandleType, VkSampler>) {
				return VK_OBJECT_TYPE_SAMPLER;
			}
			else if constexpr (std::is_same_v<VulkanHandleType, VkSamplerYcbcrConversion>) {
				return VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION;
			}
			else if constexpr (std::is_same_v<VulkanHandleType, VkPipelineCache>) {
				return VK_OBJECT_TYPE_PIPELINE_CACHE;
			}
			else if constexpr (std::is_same_v<VulkanHandleType, VkQueryPool>) {
				return VK_OBJECT_TYPE_QUERY_POOL;
			}
			else if constexpr (std::is_same_v<VulkanHandleType, VkEvent>) {
				return VK_OBJECT_TYPE_EVENT;
			}
			else if constexpr (std::is_same_v<VulkanHandleType, VkFence>) {
				return VK_OBJECT_TYPE_FENCE;
			}
			else if constexpr (std::is_same_v<VulkanHandleType, VkSemaphore>) {
				return VK_OBJECT_TYPE_SEMAPHORE;
			}
			else {
				static_assert(false, "Unknown Vulkan object type.");
				return VK_OBJECT_TYPE_UNKNOWN;
			}
		}

		template<typename VulkanHandleType>
		static void SetDebugName(VulkanHandleType object, const std::string& name)
		{
			VkDebugUtilsObjectNameInfoEXT info{};
			info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
			info.objectType = GetVkObjectType<VulkanHandleType>();
			info.objectHandle = reinterpret_cast<uint64_t>(object);
			info.pObjectName = name.c_str();

			vkSetDebugUtilsObjectNameEXT(VkRenderer::Get().GetDevice(), &info);
		}
	};
}
