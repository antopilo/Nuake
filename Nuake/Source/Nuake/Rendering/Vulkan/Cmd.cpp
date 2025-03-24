#include "Cmd.h"

using namespace Nuake;

void Cmd::BeginRendering(VkRenderingInfo renderInfo)
{
	vkCmdBeginRendering(CmdBuffer, &renderInfo);
}

void Cmd::EndRendering()
{
	vkCmdEndRendering(CmdBuffer);
}

void Cmd::BindPipeline(VkPipeline pipeline) const
{
	vkCmdBindPipeline(CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

void Cmd::SetViewport(const Vector2 & size) const
{
	VkViewport viewport = {};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = size.x;
	viewport.height = size.y;
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;

	vkCmdSetViewport(CmdBuffer, 0, 1, &viewport);
}

void Cmd::SetScissor(const Vector2 & size) const
{
	VkRect2D scissor = {};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = size.x;
	scissor.extent.height = size.y;
	vkCmdSetScissor(CmdBuffer, 0, 1, &scissor);
}

void Cmd::ClearColorImage(Ref<VulkanImage> img, Color color) const
{
	VkClearColorValue clearValue = { {color.r, color.g, color.b, color.a } };
	VkImageSubresourceRange clearRange{};
	clearRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	clearRange.baseMipLevel = 0;
	clearRange.levelCount = VK_REMAINING_MIP_LEVELS;
	clearRange.baseArrayLayer = 0;
	clearRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
	vkCmdClearColorImage(CmdBuffer, img->GetImage(), VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);
}

void Cmd::ClearDepthImage(Ref<VulkanImage> img) const
{
	VkClearDepthStencilValue clearValue = {};
	clearValue.depth = 0.0f;
	clearValue.stencil = 0;

	VkImageSubresourceRange clearRange = {};
	clearRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	clearRange.baseMipLevel = 0;
	clearRange.levelCount = VK_REMAINING_MIP_LEVELS;
	clearRange.baseArrayLayer = 0;
	clearRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

	vkCmdClearDepthStencilImage(CmdBuffer, img->GetImage(), VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);
}

void Cmd::BindDescriptorSet(VkPipelineLayout pipeline, VkDescriptorSet descriptor, uint32_t set) const
{
	vkCmdBindDescriptorSets(
			CmdBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline,
			set,                // firstSet
			1,					// descriptorSetCount
			&descriptor,        // pointer to the descriptor set(s)
			0,                  // dynamicOffsetCount
			nullptr             // dynamicOffsets
	);
}

void Cmd::BindIndexBuffer(VkBuffer buffer) const
{
	vkCmdBindIndexBuffer(CmdBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);
}

void Cmd::DrawIndexed(uint32_t count) const
{
	vkCmdDrawIndexed(CmdBuffer, count, 1, 0, 0, 0);
}

void Cmd::PushConstants(VkPipelineLayout pipeline, size_t size, void* data) const
{
	vkCmdPushConstants(
			CmdBuffer,
			pipeline,
			VK_SHADER_STAGE_ALL_GRAPHICS,
			0,		// Offset
			size,	// Size of the push constant
			data	// Pointer to the value
	);
}

void Cmd::CopyBuffer(VkBuffer src, VkBuffer dst, size_t size) const
{
	VkBufferCopy copy{ 0 };
	copy.dstOffset = 0;
	copy.srcOffset = 0;
	copy.size = size;

	vkCmdCopyBuffer(CmdBuffer, src, dst, 1, &copy);
}

void Cmd::TransitionImageLayout(Ref<VulkanImage> img, VkImageLayout layout) const
{
	img->TransitionLayout(CmdBuffer, layout);
}

void Cmd::CopyImageToImage(Ref<VulkanImage> src, Ref<VulkanImage> dst) const
{
	VkImageBlit2 blitRegion{ .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2, .pNext = nullptr };

	blitRegion.srcOffsets[1].x = src->GetSize().x;
	blitRegion.srcOffsets[1].y = src->GetSize().y;
	blitRegion.srcOffsets[1].z = 1;

	blitRegion.dstOffsets[1].x = dst->GetSize().x;
	blitRegion.dstOffsets[1].y = dst->GetSize().y;
	blitRegion.dstOffsets[1].z = 1;

	blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blitRegion.srcSubresource.baseArrayLayer = 0;
	blitRegion.srcSubresource.layerCount = 1;
	blitRegion.srcSubresource.mipLevel = 0;

	blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blitRegion.dstSubresource.baseArrayLayer = 0;
	blitRegion.dstSubresource.layerCount = 1;
	blitRegion.dstSubresource.mipLevel = 0;

	VkBlitImageInfo2 blitInfo{ .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2, .pNext = nullptr };
	blitInfo.dstImage = dst->GetImage();
	blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	blitInfo.srcImage = src->GetImage();
	blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	blitInfo.filter = VK_FILTER_LINEAR;
	blitInfo.regionCount = 1;
	blitInfo.pRegions = &blitRegion;

	vkCmdBlitImage2(CmdBuffer, &blitInfo);
}

void Cmd::SetLineRasterizationMode(VkLineRasterizationMode mode) const
{
	vkCmdSetLineRasterizationModeEXT(CmdBuffer, mode);
}

void Cmd::SetLineWidth(float lineWidth) const
{
	vkCmdSetLineWidth(CmdBuffer, lineWidth);
}

void Cmd::SetLineStippleEnabled(bool enabled) const
{
	vkCmdSetLineStippleEnableEXT(CmdBuffer, static_cast<VkBool32>(enabled));
}

void Cmd::SetLineStipple(uint32_t factor, uint16_t pattern) const
{
	vkCmdSetLineStippleKHR(CmdBuffer, factor, pattern);
}

void Cmd::SetPrimitiveTopology(VkPrimitiveTopology topology) const
{
	vkCmdSetPrimitiveTopology(CmdBuffer, topology);
}

void Cmd::SetPolygonMode(VkPolygonMode mode) const
{
	vkCmdSetPolygonModeEXT(CmdBuffer, mode);
}


