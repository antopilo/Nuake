#include "VulkanImage.h"

#include "Nuake/Core/Logger.h"

#include "Nuake/Rendering/Vulkan/VulkanInit.h"
#include "Nuake/Rendering/Vulkan/VulkanCheck.h"
#include "Nuake/Rendering/Vulkan/VulkanAllocator.h"
#include "Nuake/Rendering/Vulkan/VulkanRenderer.h"


#include <volk/volk.h>
#include <imgui/imgui_impl_vulkan.h>

using namespace Nuake;

#include "vk_mem_alloc.h"
#include <stb_image/stb_image.h>
#include <Nuake/Rendering/Vulkan/VkResources.h>

VulkanImage::VulkanImage(const std::string & path) :
	ImGuiDescriptorSetGenerated(false),
	Format(ImageFormat::RGBA8),
	ID(UUID())
{
	int channels = 0;
	int width = 0;
	int height = 0;
	void* imageData = stbi_load(path.c_str(), &width, &height, &channels, 4);

	if (!imageData)
	{
		throw std::runtime_error("Failed to load image data using stb_image!");
	}

	// Use the loaded image data with the existing VulkanImage constructor logic
	size_t data_size = width * height * 4; // 4 for RGBA

	Ref<AllocatedBuffer> uploadBuffer = CreateRef<AllocatedBuffer>(data_size, BufferUsage::TRANSFER_SRC, MemoryUsage::CPU_TO_GPU);
	void* mappedData;
	vmaMapMemory(VulkanAllocator::Get().GetAllocator(), uploadBuffer->GetAllocation(), &mappedData);

	std::memcpy(mappedData, imageData, data_size);

	// Free the image data as we no longer need it
	stbi_image_free(imageData);

	VkExtent3D vkExtent = {
		static_cast<uint32_t>(width),
		static_cast<uint32_t>(height),
		1 // We don't support 3D images yet
	};

	Extent = { width, height, 1 };

	VkImageUsageFlags drawImageUsages = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	VkImageCreateInfo imgCreateInfo = VulkanInit::ImageCreateInfo(static_cast<VkFormat>(Format), drawImageUsages, vkExtent);

	VmaAllocationCreateInfo imgAllocInfo = {};
	imgAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	imgAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	vmaCreateImage(VulkanAllocator::Get().GetAllocator(), &imgCreateInfo, &imgAllocInfo, &Image, &Allocation, nullptr);

	VkImageViewCreateInfo imageViewCreateInfo = VulkanInit::ImageviewCreateInfo(static_cast<VkFormat>(Format), Image, VK_IMAGE_ASPECT_COLOR_BIT);

	VK_CALL(vkCreateImageView(VkRenderer::Get().GetDevice(), &imageViewCreateInfo, nullptr, &ImageView));

	// Transition image and copy data to GPU
	VkRenderer::Get().ImmediateSubmit([&](VkCommandBuffer cmd)
	{
		TransitionLayout(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		//VulkanUtil::TransitionImage(cmd, Image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkBufferImageCopy copyRegion = {};
		copyRegion.bufferOffset = 0;
		copyRegion.bufferRowLength = 0;
		copyRegion.bufferImageHeight = 0;

		copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.imageSubresource.mipLevel = 0;
		copyRegion.imageSubresource.baseArrayLayer = 0;
		copyRegion.imageSubresource.layerCount = 1;
		copyRegion.imageExtent = vkExtent;

		vkCmdCopyBufferToImage(cmd, uploadBuffer->GetBuffer(), Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

		TransitionLayout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		//VulkanUtil::TransitionImage(cmd, Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	});
}

VulkanImage::VulkanImage(ImageFormat inFormat, Vector2 inSize, ImageUsage usage) :
	Format(inFormat),
	Extent(inSize, 1),
	ImGuiDescriptorSetGenerated(false),
	ID(UUID()),
	Usage(usage)
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
		drawImageUsages |= VK_IMAGE_USAGE_SAMPLED_BIT;
		drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}

	VkImageCreateInfo imgCreateInfo = VulkanInit::ImageCreateInfo(static_cast<VkFormat>(inFormat), drawImageUsages, vkExtent);

	VmaAllocationCreateInfo imgAllocInfo = {};
	imgAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	imgAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VK_CALL(vmaCreateImage(VulkanAllocator::Get().GetAllocator(), &imgCreateInfo, &imgAllocInfo, &Image, &Allocation, nullptr));

	VkImageViewCreateInfo imageViewCreateInfo = {};
	if (usage == ImageUsage::Depth)
	{
		imageViewCreateInfo = VulkanInit::ImageviewCreateInfo(static_cast<VkFormat>(inFormat), Image, VK_IMAGE_ASPECT_DEPTH_BIT);
	}
	else
	{
		imageViewCreateInfo = VulkanInit::ImageviewCreateInfo(static_cast<VkFormat>(inFormat), Image, VK_IMAGE_ASPECT_COLOR_BIT);
	}

	VK_CALL(vkCreateImageView(VkRenderer::Get().GetDevice(), &imageViewCreateInfo, nullptr, &ImageView));

	VulkanImageCleanUpData cleanUpData
	{
		Allocation,
		ImageView,
		Image
	};

	AddGPUCleanUpFunc([=]() {
		VulkanImageCleanUpData dataCopy = cleanUpData;

		VmaAllocationInfo info;
		vmaGetAllocationInfo(VulkanAllocator::Get().GetAllocator(), dataCopy.Allocation, &info);

		if (info.pName)
		{
			Logger::Log("Deleting " + std::string(info.pName), "Vulkan", VERBOSE);
		}

		vkDestroyImageView(VkRenderer::Get().GetDevice(), dataCopy.ImageView, nullptr);
		vmaDestroyImage(VulkanAllocator::Get().GetAllocator(), dataCopy.Image, dataCopy.Allocation);
	});
}

VulkanImage::VulkanImage(void* inData, ImageFormat inFormat, Vector2 inSize) : VulkanImage(inFormat, inSize)
{
	size_t data_size = inSize.x * inSize.y;

	if (inFormat == ImageFormat::RGB16F)
	{
		data_size *= 6;
	}
	else if (inFormat == ImageFormat::RG16F)
	{
		data_size *= 4;
	}
	else if (inFormat == ImageFormat::RG32F)
	{
		data_size *= 8;
	}
	else if (inFormat != ImageFormat::A8)
	{
		data_size *= 4;
	}

	Ref<AllocatedBuffer> uploadBuffer = CreateRef<AllocatedBuffer>(data_size, BufferUsage::TRANSFER_SRC, MemoryUsage::CPU_TO_GPU);
	void* mappedData;
	vmaMapMemory(VulkanAllocator::Get().GetAllocator(), uploadBuffer->GetAllocation(), &mappedData);

	std::memcpy(mappedData, inData, data_size);

	VkExtent3D vkExtent =
	{
		static_cast<uint32_t>(inSize.x),
		static_cast<uint32_t>(inSize.y),
		1 // We dont support 3D images yet
	};

	VkImageUsageFlags drawImageUsages{};
	drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_SAMPLED_BIT;

	VkImageCreateInfo imgCreateInfo = VulkanInit::ImageCreateInfo(static_cast<VkFormat>(inFormat), drawImageUsages, vkExtent);

	VmaAllocationCreateInfo imgAllocInfo = {};
	imgAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	imgAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	vmaCreateImage(VulkanAllocator::Get().GetAllocator(), &imgCreateInfo, &imgAllocInfo, &Image, &Allocation, nullptr);

	VkImageViewCreateInfo imageViewCreateInfo;
	imageViewCreateInfo = VulkanInit::ImageviewCreateInfo(static_cast<VkFormat>(inFormat), Image, VK_IMAGE_ASPECT_COLOR_BIT);

	VK_CALL(vkCreateImageView(VkRenderer::Get().GetDevice(), &imageViewCreateInfo, nullptr, &ImageView));

	VkRenderer::Get().ImmediateSubmit([&](VkCommandBuffer cmd) 
		{
			//VulkanUtil::TransitionImage(cmd, Image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			TransitionLayout(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			VkBufferImageCopy copyRegion = {};
			copyRegion.bufferOffset = 0;
			copyRegion.bufferRowLength = 0;
			copyRegion.bufferImageHeight = 0;

			copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.imageSubresource.mipLevel = 0;
			copyRegion.imageSubresource.baseArrayLayer = 0;
			copyRegion.imageSubresource.layerCount = 1;
			copyRegion.imageExtent = vkExtent;

			vkCmdCopyBufferToImage(cmd, uploadBuffer->GetBuffer(), Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
			&copyRegion);
			TransitionLayout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			//VulkanUtil::TransitionImage(cmd, Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
	);

	AddGPUCleanUpFunc([=]() {
		//vkDestroyImageView(VkRenderer::Get().GetDevice(), ImageView, nullptr);
		//vmaDestroyImage(VulkanAllocator::Get().GetAllocator(), Image, Allocation);
	});
}

VulkanImage::VulkanImage(void* inData, size_t inSize) :
	Format(ImageFormat::RGBA8),
	ImGuiDescriptorSetGenerated(false),
	ID(UUID())
{
	int channels = 0;
	int width = 0;
	int height = 0;
	void* imageData = stbi_load_from_memory(static_cast<const stbi_uc*>(inData), static_cast<int>(inSize), &width, &height, &channels, 4);

	if (!imageData)
	{
		throw std::runtime_error("Failed to load image data using stb_image!");
	}

	// Use the loaded image data with the existing VulkanImage constructor logic
	size_t data_size = width * height * 4; // 4 for RGBA

	Ref<AllocatedBuffer> uploadBuffer = CreateRef<AllocatedBuffer>(data_size, BufferUsage::TRANSFER_SRC, MemoryUsage::CPU_TO_GPU);
	void* mappedData;
	vmaMapMemory(VulkanAllocator::Get().GetAllocator(), uploadBuffer->GetAllocation(), &mappedData);

	std::memcpy(mappedData, imageData, data_size);

	// Free the image data as we no longer need it
	stbi_image_free(imageData);

	VkExtent3D vkExtent = {
		static_cast<uint32_t>(width),
		static_cast<uint32_t>(height),
		1 // We don't support 3D images yet
	};

	Extent = { width, height, 1 };

	VkImageUsageFlags drawImageUsages = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	VkImageCreateInfo imgCreateInfo = VulkanInit::ImageCreateInfo(static_cast<VkFormat>(Format), drawImageUsages, vkExtent);

	VmaAllocationCreateInfo imgAllocInfo = {};
	imgAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	imgAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	vmaCreateImage(VulkanAllocator::Get().GetAllocator(), &imgCreateInfo, &imgAllocInfo, &Image, &Allocation, nullptr);
	

	VkImageViewCreateInfo imageViewCreateInfo = VulkanInit::ImageviewCreateInfo(static_cast<VkFormat>(Format), Image, VK_IMAGE_ASPECT_COLOR_BIT);

	VK_CALL(vkCreateImageView(VkRenderer::Get().GetDevice(), &imageViewCreateInfo, nullptr, &ImageView));

	// Transition image and copy data to GPU
	VkRenderer::Get().ImmediateSubmit([&](VkCommandBuffer cmd)
	{
		//VulkanUtil::TransitionImage(cmd, Image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		TransitionLayout(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		VkBufferImageCopy copyRegion = {};
		copyRegion.bufferOffset = 0;
		copyRegion.bufferRowLength = 0;
		copyRegion.bufferImageHeight = 0;

		copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.imageSubresource.mipLevel = 0;
		copyRegion.imageSubresource.baseArrayLayer = 0;
		copyRegion.imageSubresource.layerCount = 1;
		copyRegion.imageExtent = vkExtent;

		vkCmdCopyBufferToImage(cmd, uploadBuffer->GetBuffer(), Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
		TransitionLayout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		//VulkanUtil::TransitionImage(cmd, Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	});

	AddGPUCleanUpFunc([=]() {
		//vkDestroyImageView(VkRenderer::Get().GetDevice(), ImageView, nullptr);
		//vmaDestroyImage(VulkanAllocator::Get().GetAllocator(), Image, Allocation);
	});
}

VulkanImage::~VulkanImage()
{
	Logger::Log("Deleting VulkanImage", "vulkan", VERBOSE);
	GPUResources::Get().QueueDeletion(GetGPUCleanUpStack());
}

void VulkanImage::SetDebugName(const std::string& name)
{
	GPUManaged::SetDebugName(name);

	vmaSetAllocationName(VulkanAllocator::Get().GetAllocator(), Allocation, GetDebugName().data());
}

void VulkanImage::TransitionLayout(VkCommandBuffer cmd, VkImageLayout layout)
{
	if (Layout == layout)
	{
		return;
	}

	if(!IsLayoutTransitionValid(Layout, layout))
	{
		throw std::runtime_error("Invalid layout transition!");
	}

	VkImageMemoryBarrier2 imageBarrier{ .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
	imageBarrier.pNext = nullptr;

	imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
	imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
	imageBarrier.oldLayout = Layout;
	imageBarrier.newLayout = layout;

	VkImageAspectFlags aspectMask = (layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL || Usage == ImageUsage::Depth) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	imageBarrier.subresourceRange = VulkanInit::ImageSubResourceRange(aspectMask);
	imageBarrier.image = GetImage();

	VkDependencyInfo depInfo{};
	depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	depInfo.pNext = nullptr;

	depInfo.imageMemoryBarrierCount = 1;
	depInfo.pImageMemoryBarriers = &imageBarrier;

	vkCmdPipelineBarrier2(cmd, &depInfo);

	Layout = layout;
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

		struct cleanUpData
		{
			VkDescriptorSet descriptorSet;
			VkSampler sampler;
		};

		cleanUpData dataToCleanUp
		{
			ImGuiDescriptorSet,
			Sampler
		};

		AddGPUCleanUpFunc([=]() {
			cleanUpData data = dataToCleanUp;
			ImGui_ImplVulkan_RemoveTexture(data.descriptorSet);
			vkDestroySampler(VkRenderer::Get().GetDevice(), data.sampler, nullptr);
		});
	}

	return ImGuiDescriptorSet;
}
