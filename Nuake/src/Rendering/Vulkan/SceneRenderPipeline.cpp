#include "SceneRenderPipeline.h"

#include "src/Rendering/Vulkan/ShaderManager.h"
#include "src/Rendering/Vulkan/VkResources.h"

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
	//gBufferPass.SetPushConstant<ModelPushConstant>(modelPushConstant);

	auto& shadingPass = GBufferPipeline.AddPass("Shading");
	shadingPass.SetShaders(shaderMgr.GetShader("shading_vert"), shaderMgr.GetShader("shading_frag"));
	//shadingPass.SetPushConstant<ShadingPushConstant>(shadingPushConstant);
	shadingPass.AddAttachment("Output", ImageFormat::RGBA8);
	shadingPass.SetDepthTest(false);
	shadingPass.AddInput("Albedo");
	shadingPass.AddInput("Normal");
	shadingPass.AddInput("Depth");
	shadingPass.AddInput("Material");
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
		{ GBufferMaterial, GBufferDepth, GBufferNormal, GBufferMaterial }, // GBuffer
		{ ShadingOutput } // Shading
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
