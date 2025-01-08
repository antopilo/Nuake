#include "RenderPipeline.h"

#include "src/Rendering/Vulkan/PipelineBuilder.h"
#include "src/Rendering/Vulkan/VkResources.h"
#include "src/Rendering/Vulkan/VulkanCheck.h"
#include "src/Rendering/Vulkan/VulkanInit.h"
#include "src/Rendering/Vulkan/VulkanRenderer.h"
#include "src/Rendering/Vulkan/VulkanImage/VulkanImage.h"

using namespace Nuake;

TextureAttachment::TextureAttachment(const std::string& name, ImageFormat format) :
	Name(name),
	Format(format)
{
}

RenderPass::RenderPass(const std::string& name) :
	Name(name)
{
}

void RenderPass::ClearAttachments(PassRenderContext& ctx)
{
	// Clear all color attachments
	VkClearColorValue clearValue = { { 0.0f, 0.0f, 0.0f, 1.0f } };
	VkImageSubresourceRange clearRange = VulkanInit::ImageSubResourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
	for (auto& attachment : Attachments)
	{
		vkCmdClearColorImage(ctx.commandBuffer,attachment.Image->GetImage(), VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);
	}

	// Clear depth?
}

void RenderPass::TransitionAttachments(PassRenderContext& ctx)
{
	// Transition all color attachments
	for (auto& attachment : Attachments)
	{
		VulkanUtil::TransitionImage(ctx.commandBuffer, attachment.Image->GetImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	}

	// Transition depth attachment
	VulkanUtil::TransitionImage(ctx.commandBuffer, DepthAttachment.Image->GetImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
}

void RenderPass::Render(PassRenderContext& ctx)
{
	if (PreRender)
	{
		PreRender(ctx);
	}

	// Begin rendering and bind pipeline
	std::vector<VkRenderingAttachmentInfo> renderAttachmentInfos;
	renderAttachmentInfos.reserve(Attachments.size());

	for (auto& attachment : Attachments)
	{
		VkRenderingAttachmentInfo attachmentInfo = VulkanInit::AttachmentInfo(attachment.Image->GetImageView(), nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		renderAttachmentInfos.push_back(attachmentInfo);
	}
	
	VkRenderingAttachmentInfo depthAttachmentInfo = VulkanInit::DepthAttachmentInfo(DepthAttachment.Image->GetImageView(), VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
	
	VkRenderingInfo renderInfo = VulkanInit::RenderingInfo(ctx.resolution, renderAttachmentInfos, &depthAttachmentInfo);
	renderInfo.colorAttachmentCount = std::size(renderAttachmentInfos);
	renderInfo.pColorAttachments = renderAttachmentInfos.data();

	// Begin render!
	vkCmdBeginRendering(ctx.commandBuffer, &renderInfo);
	{
		vkCmdBindPipeline(ctx.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline);

		if (RenderCb)
		{
			RenderCb(ctx);
		}
	}
	vkCmdEndRendering(ctx.commandBuffer);
	// End rendering

	if (PostRender)
	{
		PostRender(ctx);
	}
}

TextureAttachment& RenderPass::AddAttachment(const std::string& name, ImageFormat format, ImageUsage usage)
{
	auto newAttachment = TextureAttachment(name, format);
	if (usage == ImageUsage::Depth)
	{
		DepthAttachment = newAttachment;
	}

	TextureAttachment& newAttachmentRef = Attachments.emplace_back(std::move(newAttachment));
	return newAttachmentRef;
}

void RenderPass::AddInput(const TextureAttachment& name)
{
	Inputs.push_back(name);
}

void RenderPass::SetShaders(Ref<VulkanShader> vertShader, Ref<VulkanShader> fragShader)
{
	VertShader = vertShader;
	FragShader = fragShader;
}

void RenderPass::Build()
{
	// Push constant range
	VkPushConstantRange bufferRange{};
	bufferRange.offset = 0;
	bufferRange.size = PushConstantSize;
	bufferRange.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

	// TODO: Get the bindless descriptor layout
	std::vector<VkDescriptorSetLayout> layouts = GPUResources::Get().GetBindlessLayout();

	// Create pipeline layout
	VkPipelineLayoutCreateInfo pipeline_layout_info = VulkanInit::PipelineLayoutCreateInfo();
	pipeline_layout_info.pPushConstantRanges = &bufferRange;
	pipeline_layout_info.pushConstantRangeCount = 1;
	pipeline_layout_info.pSetLayouts = layouts.data();
	pipeline_layout_info.setLayoutCount = layouts.size();

	VkPipelineLayout pipelineLayout;
	VK_CALL(vkCreatePipelineLayout(VkRenderer::Get().GetDevice(), &pipeline_layout_info, nullptr, &pipelineLayout));

	// Create pipeline 
	const size_t attachmentCount = Attachments.size();

	PipelineBuilder pipelineBuilder;
	pipelineBuilder.PipelineLayout = pipelineLayout;
	pipelineBuilder.SetShaders(VertShader->GetModule(), FragShader->GetModule());
	pipelineBuilder.SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	pipelineBuilder.SetPolygonMode(VK_POLYGON_MODE_FILL);
	pipelineBuilder.SetCullMode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE);
	pipelineBuilder.SetMultiSamplingNone();
	pipelineBuilder.EnableBlendingAlphaBlend(attachmentCount);

	// Set color attachments
	std::vector<VkFormat> formats;
	formats.reserve(attachmentCount);
	for (int i = 0; i < attachmentCount; i++)
	{
		formats.push_back(static_cast<VkFormat>(Attachments[i].Format));
	}

	// Set depth attachment, for now we assume every pass has a depth attachment
	pipelineBuilder.SetDepthFormat(static_cast<VkFormat>(DepthAttachment.Format));
	pipelineBuilder.EnableDepthTest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);
	Pipeline = pipelineBuilder.BuildPipeline(VkRenderer::Get().GetDevice());
}

void RenderPass::SetPushConstant(std::any data, size_t size)
{
	PushConstant = data;
	PushConstantSize = size;
}

RenderPipeline::RenderPipeline() :
	Built(false)
{ }

RenderPass& RenderPipeline::AddPass(const std::string& name)
{
	auto newPass = RenderPass(name);
	return RenderPasses.emplace_back(std::move(newPass));
}

void RenderPipeline::Build()
{
	for (auto& pass : RenderPasses)
	{
		pass.Build();
	}

	Built = true;
}

void RenderPipeline::Execute(PassRenderContext& ctx)
{
	if (!Built)
	{
		Logger::Log("Pipeline not built", "vulkan", CRITICAL);
		return;
	}

	for (auto& pass : RenderPasses)
	{
		pass.ClearAttachments(ctx);
		pass.TransitionAttachments(ctx);
		pass.Render(ctx);
	}
}

