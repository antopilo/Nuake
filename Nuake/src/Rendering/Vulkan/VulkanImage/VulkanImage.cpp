#include "VulkanImage.h"

#include "src/Rendering/Vulkan/VulkanInit.h"
#include "src/Rendering/Vulkan/VulkanCheck.h"
#include "src/Rendering/Vulkan/VulkanAllocator.h"
#include "src/Rendering/Vulkan/VulkanRenderer.h"

#include <volk/volk.h>
#include <imgui/imgui_impl_vulkan.h>

using namespace Nuake;

#include "vk_mem_alloc.h"

VulkanImage::VulkanImage(ImageFormat inFormat, Vector2 inSize, ImageUsage usage) :
	Format(inFormat),
	Extent(inSize, 1),
	ImGuiDescriptorSetGenerated(false)
{
	VkExtent3D vkExtent = 
	{
		static_cast<uint32_t>(inSize.x),
		static_cast<uint32_t>(inSize.y),
		1 // We dont support 3D images yet
	};

	VkImageUsageFlags drawImageUsages{};

	if (usage == ImageUsage::Default)
	{
		drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
		drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		drawImageUsages |= VK_IMAGE_USAGE_SAMPLED_BIT;
	}
	else if (usage == ImageUsage::Depth)
	{
		drawImageUsages |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}

	VkImageCreateInfo imgCreateInfo = VulkanInit::ImageCreateInfo(static_cast<VkFormat>(inFormat), drawImageUsages, vkExtent);

	VmaAllocationCreateInfo imgAllocInfo = {};
	imgAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	imgAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	vmaCreateImage(VulkanAllocator::Get().GetAllocator(), &imgCreateInfo, &imgAllocInfo, &Image, &Allocation, nullptr);

	VkImageViewCreateInfo imageViewCreateInfo;
	if (usage == ImageUsage::Depth)
	{
		imageViewCreateInfo = VulkanInit::ImageviewCreateInfo(static_cast<VkFormat>(inFormat), Image, VK_IMAGE_ASPECT_DEPTH_BIT);
	}
	else
	{
		imageViewCreateInfo = VulkanInit::ImageviewCreateInfo(static_cast<VkFormat>(inFormat), Image, VK_IMAGE_ASPECT_COLOR_BIT);
	}

	VK_CALL(vkCreateImageView(VkRenderer::Get().GetDevice(), &imageViewCreateInfo, nullptr, &ImageView));
}

VulkanImage::VulkanImage(void* inData, ImageFormat inFormat, Vector2 inSize)
{
	size_t data_size = inSize.x * inSize.y * 4;

}

VulkanImage::~VulkanImage()
{
	// TODO: deletion of image
}

VkDescriptorSet& VulkanImage::GetImGuiDescriptorSet()
{
	if (!ImGuiDescriptorSetGenerated)
	{
		VkSamplerCreateInfo sampler_info{};
		sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler_info.magFilter = VK_FILTER_LINEAR;
		sampler_info.minFilter = VK_FILTER_LINEAR;
		sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; // outside image bounds just use border color
		sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_info.minLod = -1000;
		sampler_info.maxLod = 1000;
		sampler_info.maxAnisotropy = 1.0f;
		VK_CALL(vkCreateSampler(VkRenderer::Get().GetDevice(), &sampler_info, nullptr, &Sampler));

		// Create Descriptor Set using ImGUI's implementation
		ImGuiDescriptorSet = ImGui_ImplVulkan_AddTexture(Sampler, ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		ImGuiDescriptorSetGenerated = true;
	}

	return ImGuiDescriptorSet;
}
