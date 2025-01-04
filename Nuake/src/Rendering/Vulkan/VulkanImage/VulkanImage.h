#pragma once

#include "src/Core/Maths.h"

#include "volk/volk.h"
#include "vk_mem_alloc.h"


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
		VulkanImage(ImageFormat format, Vector2 size, ImageUsage usage = ImageUsage::Default);
		VulkanImage(void* data, ImageFormat format, Vector2 size);
		VulkanImage(void* data, size_t size); // This one is for embedded files. Ignore.

		~VulkanImage();

		Vector2 GetSize() const { return Vector2(Extent.x, Extent.y); }
		int GetWidth() const { return static_cast<int>(Extent.x); }
		int GetHeight() const { return static_cast<int>(Extent.y); }

		VkImageView& GetImageView() { return ImageView; }
		VkImage& GetImage() { return Image; }
		VkDescriptorSet& GetImGuiDescriptorSet();
		ImageFormat GetFormat() const { return Format; }
	};
} 