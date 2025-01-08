#include "RenderPipeline.h"

#include "src/Rendering/Vulkan/PipelineBuilder.h"
#include "src/Rendering/Vulkan/VkResources.h"
#include "src/Rendering/Vulkan/VulkanCheck.h"
#include "src/Rendering/Vulkan/VulkanInit.h"
#include "src/Rendering/Vulkan/VulkanRenderer.h"
#include "src/Rendering/Vulkan/VulkanImage/VulkanImage.h"

using namespace Nuake;

TextureAttachment::TextureAttachment(const std::string& name, ImageFormat format, ImageUsage usage) :
	Name(name),
	Format(format)
{
	Image = std::make_shared<VulkanImage>(format, Vector2(1280, 720), usage);

	// Create default texture I guess?
	auto& gpuResources = GPUResources::Get();
	gpuResources.AddTexture(Image);
}

RenderPass::RenderPass(const std::string& name) :
	Name(name)
{
}

void RenderPass::ClearAttachments(PassRenderContext& ctx)
{
	// Resize
	for(auto& attachment : Attachments)
	{
		if (attachment.Image->GetSize() == ctx.resolution)
		{
			continue;
		}

		auto newAttachment = std::make_shared<VulkanImage>(attachment.Format, ctx.resolution);
		attachment.Image = newAttachment;

		// Register to resource manager
		auto& gpuResources = GPUResources::Get();
		gpuResources.AddTexture(newAttachment);

		// TODO: Queue deletion of old textures
	}

	if (DepthAttachment.Image->GetSize() != ctx.resolution)
	{
		Ref<VulkanImage> newDepthAttachment = std::make_shared<VulkanImage>(DepthAttachment.Format, ctx.resolution, ImageUsage::Depth);
		DepthAttachment.Image = newDepthAttachment;

		auto& gpuResources = GPUResources::Get();
		gpuResources.AddTexture(newDepthAttachment);
	}

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
	ctx.renderPass = this;

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

		VkViewport viewport = {};
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = ctx.resolution.x;
		viewport.height = ctx.resolution.y;
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;

		vkCmdSetViewport(ctx.commandBuffer, 0, 1, &viewport);

		VkRect2D scissor = {};
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		scissor.extent.width = ctx.resolution.x;
		scissor.extent.height = ctx.resolution.y;
		vkCmdSetScissor(ctx.commandBuffer, 0, 1, &scissor);

		if (RenderCb)
		{
			RenderCb(ctx);
		}
	}
	vkCmdEndRendering(ctx.commandBuffer);
	// End rendering

	for (auto& attachment : Attachments)
	{
		// Transform from color attachment to transfer src for next pass
		VulkanUtil::TransitionImage(ctx.commandBuffer, attachment.Image->GetImage(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		VulkanUtil::TransitionImage(ctx.commandBuffer, attachment.Image->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
	}

	if (PostRender)
	{
		PostRender(ctx);
	}
}

TextureAttachment& RenderPass::AddAttachment(const std::string& name, ImageFormat format, ImageUsage usage)
{
	auto newAttachment = TextureAttachment(name, format, usage);
	if (usage == ImageUsage::Depth)
	{
		DepthAttachment = newAttachment;
		return DepthAttachment;
	}

	TextureAttachment& newAttachmentRef = Attachments.emplace_back(std::move(newAttachment));
	return newAttachmentRef;
}

TextureAttachment& RenderPass::GetAttachment(const std::string& name)
{
	for (auto& attachment : Attachments)
	{
		if (attachment.Name == name)
		{
			return attachment;
		}
	}

	assert(false && "Attachment not found by name");
	return Attachments[0];
}

std::vector<TextureAttachment&> RenderPass::GetAttachments()
{
	std::vector<TextureAttachment&> attachentRefs;
	attachentRefs.reserve(Attachments.size());
	for (auto& attachment : Attachments)
	{
		attachentRefs.push_back(attachment);
	}
	return attachentRefs;
}

void RenderPass::AddInput(const std::string& name)
{
	InputNames.push_back(name);
}

std::vector<std::string> Nuake::RenderPass::GetInputs()
{
	return InputNames;
}

void RenderPass::SetInput(const std::string& name, TextureAttachment& attachment)
{
	Inputs[name] = attachment;
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

	VK_CALL(vkCreatePipelineLayout(VkRenderer::Get().GetDevice(), &pipeline_layout_info, nullptr, &PipelineLayout));

	// Create pipeline 
	const size_t attachmentCount = Attachments.size();

	PipelineBuilder pipelineBuilder;
	pipelineBuilder.PipelineLayout = PipelineLayout;
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

RenderPass& RenderPipeline::GetRenderPass(const std::string& name)
{
	// Find render pass by name
	for (auto& pass : RenderPasses)
	{
		if (pass.GetName() == name)
		{
			return pass;
		}
	}

	assert(false && "Render pass not found by name");
	return RenderPasses[0];
}

bool RenderPipeline::Build()
{
	std::map<std::string, TextureAttachment&> attachments;
	for (auto& pass : RenderPasses)
	{
		pass.Build();

		for (auto& input : pass.GetInputs())
		{
			if (attachments.find(input) == attachments.end())
			{
				Logger::Log("Failed to build RenderPipeline. input " + input + " not found in previous passes.", "vulkan", CRITICAL);
				return false;
			}

			pass.SetInput(input, attachments[input]);
		}

		for (auto& attachment : pass.GetAttachments())
		{
			attachments[attachment.Name] = attachment;
		}
	}

	for (auto& pass : RenderPasses)
	{
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

