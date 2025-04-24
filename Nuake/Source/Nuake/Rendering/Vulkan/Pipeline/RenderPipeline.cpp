#include "RenderPipeline.h"

#include "Nuake/Rendering/Vulkan/PipelineBuilder.h"
#include "Nuake/Rendering/Vulkan/VkResources.h"
#include "Nuake/Rendering/Vulkan/VulkanCheck.h"
#include "Nuake/Rendering/Vulkan/VulkanInit.h"
#include "Nuake/Rendering/Vulkan/VulkanRenderer.h"
#include "Nuake/Rendering/Vulkan/VulkanImage/VulkanImage.h"

using namespace Nuake;

TextureAttachment::TextureAttachment(const std::string& name, ImageFormat format, ImageUsage usage, bool clearOnLoad) :
	Name(name),
	Format(format),
	ClearOnLoad(clearOnLoad)
{
	Image = std::make_shared<VulkanImage>(format, Vector2(1280, 720), usage);

	// Create default texture I guess?
	auto& gpuResources = GPUResources::Get();
	gpuResources.AddTexture(Image);
}

TextureAttachment::~TextureAttachment()
{
	if (Image)
	{
		auto& gpuResources = GPUResources::Get();
		gpuResources.RemoveTexture(Image);
	}
}

RenderPass::RenderPass(const std::string& name) :
	Name(name),
	PushConstantSize(0),
	IsLinePass(false),
	Topology(PolygonTopology::TRIANGLE_LIST),
	IsBuilt(false)
{
}

RenderPass::~RenderPass()
{
	if (!IsBuilt)
	{
		return;
	}

	vkDestroyPipeline(VkRenderer::Get().GetDevice(), Pipeline, nullptr);
	vkDestroyPipelineLayout(VkRenderer::Get().GetDevice(), PipelineLayout, nullptr);
}

void RenderPass::SetIsLinePass(bool enabled)
{
	IsLinePass = enabled;
}

void RenderPass::Execute(PassRenderContext& ctx, PassAttachments& inputs)
{
	ctx.commandBuffer.SetPolygonMode(VK_POLYGON_MODE_FILL);
	ctx.commandBuffer.SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	ctx.commandBuffer.SetLineRasterizationMode(VK_LINE_RASTERIZATION_MODE_DEFAULT);
	ctx.commandBuffer.SetLineStippleEnabled(false);

	ClearAttachments(ctx, inputs);
	TransitionAttachments(ctx, inputs);
	Render(ctx, inputs);
	UntransitionAttachments(ctx, inputs);
}

void RenderPass::ClearAttachments(PassRenderContext& ctx, PassAttachments& inputs)
{
	// Clear all color attachments
	int attachmentIndex = 0;
	for (int i = 0; i < std::size(inputs); i++)
	{
		Ref<VulkanImage> input = inputs[i];
		if (input->GetUsage() == ImageUsage::Depth)
		{
			if (DepthAttachment.ClearOnLoad)
			{
				ctx.commandBuffer.ClearDepthImage(input);
			}
		}
		else if(input->GetUsage() != ImageUsage::Depth)
		{
			if (Attachments[attachmentIndex].ClearOnLoad)
			{
				ctx.commandBuffer.ClearColorImage(input, ClearColor);
			}

			attachmentIndex++;
		}
	}
}

void RenderPass::TransitionAttachments(PassRenderContext& ctx, PassAttachments& inputs)
{
	// Transition all color attachments
	for (auto& attachment : inputs)
	{
		if (attachment->GetUsage() != ImageUsage::Depth)
		{
			ctx.commandBuffer.TransitionImageLayout(attachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		}
	}
}

void RenderPass::UntransitionAttachments(PassRenderContext& ctx, PassAttachments& inputs)
{
	for (auto& attachment : inputs)
	{
		// Transform from color attachment to transfer src for next pass
		ctx.commandBuffer.TransitionImageLayout(attachment, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		ctx.commandBuffer.TransitionImageLayout(attachment, VK_IMAGE_LAYOUT_GENERAL);
	}
}

void RenderPass::Render(PassRenderContext& ctx, PassAttachments& inputs)
{
	ctx.renderPass = this;

	if (PreRender)
	{
		PreRender(ctx);
	}
	
	// Begin rendering and bind pipeline
	std::vector<VkRenderingAttachmentInfo> renderAttachmentInfos;
	VkRenderingAttachmentInfo depthAttachmentInfo = {};
	renderAttachmentInfos.reserve(Attachments.size());
	bool hasDepthAttachment = false;
	for (auto& attachment : inputs)
	{
		if (attachment->GetUsage() != ImageUsage::Depth)
		{
			VkRenderingAttachmentInfo attachmentInfo = VulkanInit::AttachmentInfo(attachment->GetImageView(), nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			renderAttachmentInfos.push_back(attachmentInfo);
		}
		else
		{
			depthAttachmentInfo = VulkanInit::DepthAttachmentInfo(attachment->GetImageView(), VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
			hasDepthAttachment = true;
		}
	}

	Vector2 resolution = ctx.resolution * RenderScale;
	resolution.x = static_cast<float>(static_cast<int>(resolution.x));
	resolution.y = static_cast<float>(static_cast<int>(resolution.y));
	resolution = glm::clamp(resolution, Vector2(1, 1), ctx.resolution);

	VkRenderingInfo renderInfo = VulkanInit::RenderingInfo(resolution, renderAttachmentInfos, !hasDepthAttachment ? nullptr : &depthAttachmentInfo);
	renderInfo.colorAttachmentCount = std::size(renderAttachmentInfos);
	renderInfo.pColorAttachments = renderAttachmentInfos.data();

	auto& cmd = ctx.commandBuffer;
	cmd.BeginRendering(renderInfo);
	{
		cmd.BindPipeline(Pipeline);
		cmd.SetViewport(resolution);
		cmd.SetScissor(resolution);

		if (RenderCb)
		{
			RenderCb(ctx);
		}
	}
	cmd.EndRendering();

	if (PostRender)
	{
		PostRender(ctx);
	}
}

TextureAttachment& RenderPass::AddAttachment(const std::string& name, ImageFormat format, ImageUsage usage, bool clearOnLoad)
{
	auto newAttachment = TextureAttachment(name, format, usage, clearOnLoad);
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

std::vector<TextureAttachment> RenderPass::GetAttachments()
{
	std::vector<TextureAttachment> attachentRefs;
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

std::vector<std::string> RenderPass::GetInputs()
{
	return InputNames;
}

std::vector<TextureAttachment> RenderPass::GetInputAttachments()
{
	std::vector<TextureAttachment> inputAttachments;
	inputAttachments.reserve(Inputs.size());
	for (auto& [name, attachment] : Inputs)
	{
		inputAttachments.push_back(attachment);
	}
	return inputAttachments;
}

void RenderPass::SetInput(const std::string& name, TextureAttachment attachment)
{
	Inputs[name] = attachment;
}

void RenderPass::SetShaders(Ref<VulkanShader> vertShader, Ref<VulkanShader> fragShader)
{
	VertShader = vertShader;
	FragShader = fragShader;
}

void RenderPass::SetClearColor(const Color& clearColor)
{
	this->ClearColor = clearColor;
}

void RenderPass::SetTopology(PolygonTopology topology)
{
	this->Topology = topology;
}

void RenderPass::Build()
{
	// Push constant range
	uint32_t pushRange = 0;
	VkPushConstantRange bufferRange{};
	if (PushConstantSize > 0)
	{
		bufferRange.offset = 0;
		bufferRange.size = PushConstantSize;
		bufferRange.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
		pushRange = 1;
	}

	// TODO: Get the bindless descriptor layout
	std::vector<VkDescriptorSetLayout> layouts = GPUResources::Get().GetBindlessLayout();

	// Create pipeline layout
	VkPipelineLayoutCreateInfo pipeline_layout_info = VulkanInit::PipelineLayoutCreateInfo();
	pipeline_layout_info.pPushConstantRanges = &bufferRange;
	pipeline_layout_info.pushConstantRangeCount = pushRange;
	pipeline_layout_info.pSetLayouts = layouts.data();
	pipeline_layout_info.setLayoutCount = layouts.size();

	VK_CALL(vkCreatePipelineLayout(VkRenderer::Get().GetDevice(), &pipeline_layout_info, nullptr, &PipelineLayout));

	// Create pipeline 
	const size_t attachmentCount = Attachments.size();

	PipelineBuilder pipelineBuilder;
	pipelineBuilder.PipelineLayout = PipelineLayout;

	assert(VertShader != nullptr && FragShader != nullptr && "No shader set for renderpass!");
	pipelineBuilder.SetShaders(VertShader->GetModule(), FragShader->GetModule());
	pipelineBuilder.SetInputTopology(static_cast<VkPrimitiveTopology>(Topology));
	pipelineBuilder.SetPolygonMode(VK_POLYGON_MODE_FILL);
	pipelineBuilder.SetCullMode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE);
	pipelineBuilder.SetMultiSamplingNone();
	pipelineBuilder.EnableBlendingAlphaBlend(attachmentCount);

	if (IsLinePass)
	{
		pipelineBuilder.SetLineRendering(true);
	}

	// Set color attachments
	std::vector<VkFormat> formats;
	formats.reserve(attachmentCount);
	for (int i = 0; i < attachmentCount; i++)
	{
		formats.push_back(static_cast<VkFormat>(Attachments[i].Format));
	}
	pipelineBuilder.SetColorAttachments(formats);

	// Set depth attachment, for now we assume every pass has a depth attachment
	if (HasDepthTest)
	{
		pipelineBuilder.SetDepthFormat(static_cast<VkFormat>(DepthAttachment.Format));
		pipelineBuilder.EnableDepthTest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);
	}

	Pipeline = pipelineBuilder.BuildPipeline(VkRenderer::Get().GetDevice());

	IsBuilt = true;
}

void RenderPass::SetPushConstant(std::any data, size_t size)
{
	PushConstant = data;
	PushConstantSize = size;
}

RenderPipeline::RenderPipeline() :
	Built(false)
{ }

RenderPipeline::~RenderPipeline()
{
	if (Built)
	{
		
	}
}

RenderPass& RenderPipeline::AddPass(const std::string& name)
{
	return RenderPasses.emplace_back(name);
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
	std::map<std::string, TextureAttachment> attachments;
	for (auto& pass : RenderPasses)
	{
		pass.Build();
	}

	Built = true;
	return true;
}

void RenderPipeline::Execute(PassRenderContext& ctx, PipelineAttachments& inputs)
{
	if (!Built)
	{
		Logger::Log("Pipeline not built", "vulkan", CRITICAL);
		return;
	}

	assert(std::size(RenderPasses) == std::size(inputs) && "Did you forget to an input?");

	int passIndex = 0;
	for (auto& pass : RenderPasses)
	{
		pass.Execute(ctx, inputs[passIndex]);

		passIndex++;
	}
}

