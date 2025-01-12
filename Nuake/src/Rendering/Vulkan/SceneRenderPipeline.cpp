#include "SceneRenderPipeline.h"

#include "src/Rendering/Vulkan/VkShaderManager.h"
#include "src/Rendering/Vulkan/VkResources.h"
#include <Tracy.hpp>

using namespace Nuake;

SceneRenderPipeline::SceneRenderPipeline()
{
	VkShaderManager& shaderMgr = VkShaderManager::Get();

	GBufferPipeline = RenderPipeline();

	auto& gBufferPass = GBufferPipeline.AddPass("GBuffer");
	gBufferPass.SetShaders(shaderMgr.GetShader("basic_vert"), shaderMgr.GetShader("basic_frag"));
	gBufferPass.AddAttachment("Albedo", ImageFormat::RGBA8);
	gBufferPass.AddAttachment("Normal", ImageFormat::RGBA8);
	gBufferPass.AddAttachment("Material", ImageFormat::RGBA8);
	gBufferPass.AddAttachment("Depth", ImageFormat::D32F, ImageUsage::Depth);
	gBufferPass.SetPreRender([&](PassRenderContext& ctx) {
		auto layout = ctx.renderPass->PipelineLayout;
		auto vk = VkRenderer::Get();
		ctx.commandBuffer.BindDescriptorSet(layout, vk.CameraBufferDescriptors, 0);
		//ctx.commandBuffer.BindDescriptorSet(layout, ModelBufferDescriptor, 1);
		//ctx.commandBuffer.BindDescriptorSet(layout, SamplerDescriptor, 3);
		//ctx.commandBuffer.BindDescriptorSet(layout, MaterialBufferDescriptor, 4);
		//ctx.commandBuffer.BindDescriptorSet(layout, GPUResources::Get().TextureDescriptor, 5);
		//ctx.commandBuffer.BindDescriptorSet(layout, LightBufferDescriptor, 6);
		//ctx.commandBuffer.BindDescriptorSet(layout, GPUResources::Get().CamerasDescriptor, 7);
	});
	gBufferPass.SetRender([&](PassRenderContext& ctx) {
		
	});
	//gBufferPass.SetPushConstant<ModelPushConstant>(modelPushConstant);

	auto& shadingPass = GBufferPipeline.AddPass("Shading");
	shadingPass.SetShaders(shaderMgr.GetShader("shading_vert"), shaderMgr.GetShader("shading_frag"));
	//shadingPass.SetPushConstant<ShadingPushConstant>(ShadingPushConstant()));
	shadingPass.AddAttachment("Output", ImageFormat::RGBA8);
	shadingPass.SetDepthTest(false);
	shadingPass.AddInput("Albedo");
	shadingPass.AddInput("Normal");
	shadingPass.AddInput("Depth");
	shadingPass.AddInput("Material");

	GBufferPipeline.Build();
}

void SceneRenderPipeline::SetCamera(UUID camera)
{
	CurrentCameraID = camera;
}

void SceneRenderPipeline::Render(PassRenderContext& ctx)
{
	// Resize textures
	ResizeImage(GBufferAlbedo, ctx.resolution);
	ResizeImage(GBufferDepth, ctx.resolution);
	ResizeImage(GBufferNormal, ctx.resolution);
	ResizeImage(GBufferMaterial, ctx.resolution);
	ResizeImage(ShadingOutput, ctx.resolution);

	PipelineAttachments pipelineInputs
	{
		{ GBufferMaterial, GBufferDepth, GBufferNormal, GBufferMaterial },	// GBuffer
		{ ShadingOutput }													// Shading
		// ... other passes
	};

	ctx.cameraID = CurrentCameraID;
	GBufferPipeline.Execute(ctx, pipelineInputs);
}

Ref<VulkanImage> SceneRenderPipeline::ResizeImage(Ref<VulkanImage> image, const Vector2& size)
{
	if (image->GetSize() == size)
	{
		return image;
	}

	Ref<VulkanImage> newAttachment = std::make_shared<VulkanImage>(image->GetFormat(), size);
	
	// Register to resource manager
	GPUResources& gpuResources = GPUResources::Get();
	gpuResources.AddTexture(newAttachment);

	// We might need to do this?
	// newAttachment->TransitionLayout(ctx.commandBuffer, VK_IMAGE_LAYOUT_GENERAL);
	return newAttachment;
}
