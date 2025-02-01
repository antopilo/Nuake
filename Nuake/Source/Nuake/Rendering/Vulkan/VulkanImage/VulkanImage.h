#pragma once

#include "Nuake/Core/Maths.h"
#include "Nuake/Resource/UUID.h"

#include "Nuake/Rendering/Vulkan/GPUManaged.h"

#include "volk/volk.h"
#include "vk_mem_alloc.h"

#include <string>

namespace Nuake
{
	enum class ImageFormat
	{
		A8 = 9,
		RGBA8 = 37,
		RGBA16F = 97,
		RGBA32F = 109,
		D32F = 126,
	};

	enum class ImageUsage
	{
		TransferSrc,
		TransferDst,
		Sampled,
		Storage,
		ColorAttachment,
		DepthStencilAttachment,
		TransientAttachment,
		InputAttachment,
		Depth,
		Default
	};

	struct VulkanImageCleanUpData
	{
		VmaAllocation Allocation;
		VkImageView ImageView;
		VkImage Image;
	};

	class VulkanImage : public GPUManaged
	{
	private:
		UUID ID;
		VkImage Image;
		VkImageView ImageView;
		VmaAllocation Allocation;
		Vector3 Extent;
		ImageFormat Format;
		VkSampler Sampler;
		VkImageLayout Layout = VK_IMAGE_LAYOUT_UNDEFINED;
		bool ImGuiDescriptorSetGenerated;
		VkDescriptorSet ImGuiDescriptorSet;
		ImageUsage Usage;

	public:
		VulkanImage(const std::string& path);
		VulkanImage(ImageFormat format, Vector2 size, ImageUsage usage = ImageUsage::Default);
		VulkanImage(void* data, ImageFormat format, Vector2 size);
		VulkanImage(void* data, size_t size); // This one is for embedded files. Ignore.

		~VulkanImage();

	public:
		void SetDebugName(const std::string& name) override;

		void TransitionLayout(VkCommandBuffer cmd, VkImageLayout layout);

		Vector2 GetSize() const { return Vector2(Extent.x, Extent.y); }
		int GetWidth() const { return static_cast<int>(Extent.x); }
		int GetHeight() const { return static_cast<int>(Extent.y); }

		UUID GetID() const { return ID; }
		VkImageView& GetImageView() { return ImageView; }
		VkImage GetImage() { return Image; }
		VkDescriptorSet& GetImGuiDescriptorSet();
		ImageFormat GetFormat() const { return Format; }
		VkImageLayout GetLayout() const { return Layout; }
		void SetLayout(VkImageLayout layout) { Layout = layout; }
		ImageUsage GetUsage() const { return Usage; }
		bool IsLayoutTransitionValid(VkImageLayout oldLayout, VkImageLayout newLayout) {
			switch (oldLayout) {
			case VK_IMAGE_LAYOUT_UNDEFINED:
				// Any layout is valid after VK_IMAGE_LAYOUT_UNDEFINED
				return true;

			case VK_IMAGE_LAYOUT_PREINITIALIZED:
				// Only certain layouts are valid after VK_IMAGE_LAYOUT_PREINITIALIZED
				return newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL ||
					newLayout == VK_IMAGE_LAYOUT_GENERAL;

			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				// Valid transitions for color attachments
				return newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR ||
					newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				// Valid transitions for transfer source
				return newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL ||
					newLayout == VK_IMAGE_LAYOUT_GENERAL;

			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				// Valid transitions for transfer destination
				return newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL ||
					newLayout == VK_IMAGE_LAYOUT_GENERAL;

			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				// Valid transitions for shader read-only
				return newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL ||
					newLayout == VK_IMAGE_LAYOUT_GENERAL;

			case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
				// Only transition to COLOR_ATTACHMENT_OPTIMAL for reuse
				return newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			case VK_IMAGE_LAYOUT_GENERAL:
				// General layout can transition to almost anything
				return newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ||
					newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL ||
					newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL ||
					newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL ||
					newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			default:
				// Add other cases as needed
				return false;
			}
		}
	};
} 