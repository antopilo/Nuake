#pragma once

#include "src/Core/Maths.h"

#include "volk/volk.h"
#include "vk_mem_alloc.h"


namespace Nuake
{
	enum class ImageFormat
	{
		RGBA8 = 41,
		RGBA16F = 97
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
		InputAttachment
	};

	class VulkanImage
	{
	private:
		VkImage Image;
		VkImageView ImageView;
		VmaAllocation Allocation;
		Vector3 Extent;
		ImageFormat Format;
		VkSampler Sampler;

		bool ImGuiDescriptorSetGenerated;
		VkDescriptorSet ImGuiDescriptorSet;

	public:
		VulkanImage(ImageFormat format, Vector2 size);
		VulkanImage(void* data, ImageFormat format, Vector2 size);
		~VulkanImage();

		Vector2 GetSize() const { return Vector2(Extent.x, Extent.y); }
		VkImageView& GetImageView() { return ImageView; }
		VkImage& GetImage() { return Image; }
		VkDescriptorSet& GetImGuiDescriptorSet();
		ImageFormat GetFormat() const { return Format; }
	};
} 