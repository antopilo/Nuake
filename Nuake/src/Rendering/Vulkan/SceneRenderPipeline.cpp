#include "SceneRenderPipeline.h"

#include "src/Rendering/Vulkan/VkShaderManager.h"
#include "src/Rendering/Vulkan/VkResources.h"

#include "src/Scene/Scene.h"
#include "src/Scene/Entities/Entity.h"
#include "src/Scene/Components/TransformComponent.h"
#include "src/Scene/Components/ModelComponent.h"

#include "src/Rendering/Textures/Material.h"

#include <Tracy.hpp>


using namespace Nuake;

RenderPipeline SceneRenderPipeline::GBufferPipeline;

SceneRenderPipeline::SceneRenderPipeline()
{
	// Initialize render targets
	const Vector2 defaultSize = { 1280, 720 };
	GBufferAlbedo = std::make_shared<VulkanImage>(ImageFormat::RGBA16F, defaultSize);
	GBufferNormal = std::make_shared<VulkanImage>(ImageFormat::RGBA16F, defaultSize);
	GBufferMaterial = std::make_shared<VulkanImage>(ImageFormat::RGBA8, defaultSize);
	GBufferDepth = std::make_shared<VulkanImage>(ImageFormat::D32F, defaultSize, ImageUsage::Depth);
	ShadingOutput = std::make_shared<VulkanImage>(ImageFormat::RGBA8, defaultSize);

	// Initialize pipeline
	VkShaderManager& shaderMgr = VkShaderManager::Get();

	GBufferPipeline = RenderPipeline();
	auto& gBufferPass = GBufferPipeline.AddPass("GBuffer");
	gBufferPass.SetShaders(shaderMgr.GetShader("basic_vert"), shaderMgr.GetShader("basic_frag"));
	gBufferPass.AddAttachment("Albedo", GBufferAlbedo->GetFormat());
	gBufferPass.AddAttachment("Normal", GBufferNormal->GetFormat());
	gBufferPass.AddAttachment("Material", GBufferMaterial->GetFormat());
	gBufferPass.AddAttachment("Depth", GBufferDepth->GetFormat(), ImageUsage::Depth);
	gBufferPass.SetPushConstant<GBufferConstant>(gbufferConstant);
	gBufferPass.SetPreRender([&](PassRenderContext& ctx) {
		auto& layout = ctx.renderPass->PipelineLayout;
		auto& gpu = GPUResources::Get();
		ctx.commandBuffer.BindDescriptorSet(layout, gpu.ModelDescriptor, 0);
		ctx.commandBuffer.BindDescriptorSet(layout, gpu.SamplerDescriptor, 2);
		ctx.commandBuffer.BindDescriptorSet(layout, gpu.MaterialDescriptor, 3);
		ctx.commandBuffer.BindDescriptorSet(layout, gpu.TexturesDescriptor, 4);
		ctx.commandBuffer.BindDescriptorSet(layout, gpu.LightsDescriptor, 5);
		ctx.commandBuffer.BindDescriptorSet(layout, gpu.CamerasDescriptor, 6);
	});
	gBufferPass.SetRender([&](PassRenderContext& ctx) {
		auto& cmd = ctx.commandBuffer;
		auto& scene = ctx.scene;
		auto& vk = VkRenderer::Get();
		auto& gpu = GPUResources::Get();

		ZoneScopedN("Render Models");

		gbufferConstant.CameraID = ctx.cameraID;

		auto view = scene->m_Registry.view<TransformComponent, ModelComponent, VisibilityComponent>();
		for (auto e : view)
		{
			auto [transform, mesh, visibility] = view.get<TransformComponent, ModelComponent, VisibilityComponent>(e);
			if (!mesh.ModelResource || !visibility.Visible)
			{
				continue;
			}

			gbufferConstant.Index = gpu.ModelMatrixMapping[Entity((entt::entity)e, scene.get()).GetID()];

			for (auto& m : mesh.ModelResource->GetMeshes())
			{
				Ref<VkMesh> vkMesh = m->GetVkMesh();

				cmd.BindDescriptorSet(ctx.renderPass->PipelineLayout, vkMesh->GetDescriptorSet(), 1);
				gbufferConstant.MaterialIndex = gpu.MeshMaterialMapping[vkMesh->GetID()];

				cmd.PushConstants(ctx.renderPass->PipelineLayout, sizeof(GBufferConstant), &gbufferConstant);
				cmd.BindIndexBuffer(vkMesh->GetIndexBuffer()->GetBuffer());
				cmd.DrawIndexed(vkMesh->GetIndexBuffer()->GetSize() / sizeof(uint32_t));
			}
		}
	});

	//auto& shadingPass = GBufferPipeline.AddPass("Shading");
	//shadingPass.SetShaders(shaderMgr.GetShader("shading_vert"), shaderMgr.GetShader("shading_frag"));
	//shadingPass.SetPushConstant<ShadingConstant>(shadingConstant);
	//shadingPass.AddAttachment("Output", ImageFormat::RGBA8);
	//shadingPass.SetDepthTest(false);
	//shadingPass.AddInput("Albedo");
	//shadingPass.AddInput("Normal");
	//shadingPass.AddInput("Depth");
	//shadingPass.AddInput("Material");

	GBufferPipeline.Build();
}

void SceneRenderPipeline::SetCamera(UUID camera)
{
	CurrentCameraID = camera;
}

void SceneRenderPipeline::Render(PassRenderContext& ctx)
{
	// Resize textures
	GBufferAlbedo = ResizeImage(GBufferAlbedo, ctx.resolution);
	GBufferDepth = ResizeImage(GBufferDepth, ctx.resolution);
	GBufferNormal = ResizeImage(GBufferNormal, ctx.resolution);
	GBufferMaterial = ResizeImage(GBufferMaterial, ctx.resolution);
	ShadingOutput = ResizeImage(ShadingOutput, ctx.resolution);

	PipelineAttachments pipelineInputs
	{
		{ GBufferAlbedo, GBufferDepth, GBufferNormal, GBufferMaterial },	// GBuffer
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

	Ref<VulkanImage> newAttachment = std::make_shared<VulkanImage>(image->GetFormat(), size, image->GetUsage());
	
	// Register to resource manager
	GPUResources& gpuResources = GPUResources::Get();
	gpuResources.AddTexture(newAttachment);

	// We might need to do this?
	// newAttachment->TransitionLayout(ctx.commandBuffer, VK_IMAGE_LAYOUT_GENERAL);
	return newAttachment;
}
