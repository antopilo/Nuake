#include "RenderPipeline.h"

#include "src/Rendering/Vulkan/PipelineBuilder.h"
#include "src/Rendering/Vulkan/VulkanCheck.h"
#include "src/Rendering/Vulkan/VulkanInit.h"
#include "src/Rendering/Vulkan/VulkanRenderer.h"

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
	VkPushConstantRange bufferRange{};
	bufferRange.offset = 0;
	bufferRange.size = 128;		// For now we assume we use 128 bytes
	bufferRange.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

	// TODO: Get the bindless descriptor layout
	std::vector<VkDescriptorSetLayout> layouts = { 0 };

	VkPipelineLayoutCreateInfo pipeline_layout_info = VulkanInit::PipelineLayoutCreateInfo();
	pipeline_layout_info.pPushConstantRanges = &bufferRange;
	pipeline_layout_info.pushConstantRangeCount = 1;
	pipeline_layout_info.pSetLayouts = layouts.data();
	pipeline_layout_info.setLayoutCount = layouts.size();

	VkPipelineLayout pipelineLayout;
	VK_CALL(vkCreatePipelineLayout(VkRenderer::Get().GetDevice(), &pipeline_layout_info, nullptr, &pipelineLayout));

	PipelineBuilder pipelineBuilder;
	pipelineBuilder.PipelineLayout = pipelineLayout;
	pipelineBuilder.SetShaders(VertShader->GetModule(), FragShader->GetModule());
	pipelineBuilder.SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	pipelineBuilder.SetPolygonMode(VK_POLYGON_MODE_FILL);
	pipelineBuilder.SetCullMode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE);
	pipelineBuilder.SetMultiSamplingNone();

	for (int i = 0; i < Attachments.size(); i++)
	{
		pipelineBuilder.EnableBlendingAlphaBlend();
	}

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

void RenderPipeline::Execute(std::span<std::string> inputs)
{

}

