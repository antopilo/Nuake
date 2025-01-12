#include "SceneRenderPipeline.h"
#include "VkResources.h"

using namespace Nuake;

SceneRenderPipeline::SceneRenderPipeline(UUID camera) :
	CurrentCameraID(camera)
{

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
