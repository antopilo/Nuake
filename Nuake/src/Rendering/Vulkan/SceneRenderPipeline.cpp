#include "SceneRenderPipeline.h"

#include "src/Rendering/Textures/Material.h"
#include "src/Rendering/Vulkan/VkShaderManager.h"
#include "src/Rendering/Vulkan/VkResources.h"

#include "src/Scene/Scene.h"
#include "src/Scene/Entities/Entity.h"
#include "src/Scene/Components/TransformComponent.h"
#include "src/Scene/Components/ModelComponent.h"

#include <Tracy.hpp>

using namespace Nuake;

RenderPipeline SceneRenderPipeline::GBufferPipeline;
RenderPipeline SceneRenderPipeline::ShadowPipeline;
SceneRenderPipeline::SceneRenderPipeline()
{
	// Initialize render targets
	const Vector2 defaultSize = { 1, 1 };
	GBufferAlbedo = CreateRef<VulkanImage>(ImageFormat::RGBA16F, defaultSize);
	GBufferNormal = CreateRef<VulkanImage>(ImageFormat::RGBA16F, defaultSize);
	GBufferMaterial = CreateRef<VulkanImage>(ImageFormat::RGBA8, defaultSize);
	GBufferDepth = CreateRef<VulkanImage>(ImageFormat::D32F, defaultSize, ImageUsage::Depth);

	ShadingOutput = CreateRef<VulkanImage>(ImageFormat::RGBA8, defaultSize);

	// Initialize pipeline
	VkShaderManager& shaderMgr = VkShaderManager::Get();

	ShadowPipeline = RenderPipeline();
	auto& shadowPass = ShadowPipeline.AddPass("Shadow");
	shadowPass.AddAttachment("Depth", ImageFormat::D32F, ImageUsage::Depth);
	shadowPass.SetShaders(shaderMgr.GetShader("shadow_vert"), shaderMgr.GetShader("shadow_frag"));
	shadowPass.SetPushConstant<GBufferConstant>(gbufferConstant);
	shadowPass.SetPreRender([&](PassRenderContext& ctx) {
		auto& layout = ctx.renderPass->PipelineLayout;
		auto& res = GPUResources::Get();

		Cmd& cmd = ctx.commandBuffer;
		cmd.BindDescriptorSet(layout, res.ModelDescriptor, 0);
		cmd.BindDescriptorSet(layout, res.SamplerDescriptor, 2);
		cmd.BindDescriptorSet(layout, res.MaterialDescriptor, 3);
		cmd.BindDescriptorSet(layout, res.TexturesDescriptor, 4);
		cmd.BindDescriptorSet(layout, res.LightsDescriptor, 5);
		cmd.BindDescriptorSet(layout, res.CamerasDescriptor, 6);
	});
	shadowPass.SetRender([&](PassRenderContext& ctx) {
		auto& cmd = ctx.commandBuffer;
		auto& scene = ctx.scene;
		auto& vk = VkRenderer::Get();
	
		ZoneScopedN("Render Models");
		auto view = scene->m_Registry.view<TransformComponent, ModelComponent, VisibilityComponent>();
		for (auto e : view)
		{
			auto [transform, mesh, visibility] = view.get<TransformComponent, ModelComponent, VisibilityComponent>(e);
			if (!mesh.ModelResource || !visibility.Visible)
			{
				continue;
			}
	
			const int entityId = Entity((entt::entity)e, scene.get()).GetID();
			for (auto& m : mesh.ModelResource->GetMeshes())
			{
				Ref<VkMesh> vkMesh = m->GetVkMesh();
				cmd.BindDescriptorSet(ctx.renderPass->PipelineLayout, vkMesh->GetDescriptorSet(), 1);

				gbufferConstant.Index = GPUResources::Get().GetBindlessTransformID(entityId);
				gbufferConstant.CameraID = ctx.cameraID;
	
				cmd.PushConstants(ctx.renderPass->PipelineLayout, sizeof(GBufferConstant), &gbufferConstant);
				cmd.BindIndexBuffer(vkMesh->GetIndexBuffer()->GetBuffer());
				cmd.DrawIndexed(vkMesh->GetIndexBuffer()->GetSize() / sizeof(uint32_t));
			}
		}
	});

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
		auto& res = GPUResources::Get();

		Cmd& cmd = ctx.commandBuffer;
		cmd.BindDescriptorSet(layout, res.ModelDescriptor, 0);
		cmd.BindDescriptorSet(layout, res.SamplerDescriptor, 2);
		cmd.BindDescriptorSet(layout, res.MaterialDescriptor, 3);
		cmd.BindDescriptorSet(layout, res.TexturesDescriptor, 4);
		cmd.BindDescriptorSet(layout, res.LightsDescriptor, 5);
		cmd.BindDescriptorSet(layout, res.CamerasDescriptor, 6);
	});
	gBufferPass.SetRender([&](PassRenderContext& ctx) {
		Cmd& cmd = ctx.commandBuffer;
		Ref<Scene> scene = ctx.scene;
		auto& res = GPUResources::Get();

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

			gbufferConstant.Index = res.ModelMatrixMapping[Entity((entt::entity)e, scene.get()).GetID()];

			for (auto& m : mesh.ModelResource->GetMeshes())
			{
				Ref<VkMesh> vkMesh = m->GetVkMesh();

				cmd.BindDescriptorSet(ctx.renderPass->PipelineLayout, vkMesh->GetDescriptorSet(), 1);
				gbufferConstant.MaterialIndex = res.MeshMaterialMapping[vkMesh->GetID()];

				cmd.PushConstants(ctx.renderPass->PipelineLayout, sizeof(GBufferConstant), &gbufferConstant);
				cmd.BindIndexBuffer(vkMesh->GetIndexBuffer()->GetBuffer());
				cmd.DrawIndexed(vkMesh->GetIndexBuffer()->GetSize() / sizeof(uint32_t));
			}
		}
	});

	auto& shadingPass = GBufferPipeline.AddPass("Shading");
	shadingPass.SetShaders(shaderMgr.GetShader("shading_vert"), shaderMgr.GetShader("shading_frag"));
	shadingPass.SetPushConstant<ShadingConstant>(shadingConstant);
	shadingPass.AddAttachment("Output", ShadingOutput->GetFormat());
	shadingPass.SetDepthTest(false);
	shadingPass.AddInput("Albedo");
	shadingPass.AddInput("Normal");
	shadingPass.AddInput("Depth");
	shadingPass.AddInput("Material");
	shadingPass.SetPreRender([&](PassRenderContext& ctx) {
		Cmd& cmd = ctx.commandBuffer;
		auto& layout = ctx.renderPass->PipelineLayout;
		auto& res = GPUResources::Get();

		// Bindless
		cmd.BindDescriptorSet(layout, res.ModelDescriptor, 0);
		cmd.BindDescriptorSet(layout, res.SamplerDescriptor, 2);
		cmd.BindDescriptorSet(layout, res.MaterialDescriptor, 3);
		cmd.BindDescriptorSet(layout, res.TexturesDescriptor, 4);
		cmd.BindDescriptorSet(layout, res.LightsDescriptor, 5);
		cmd.BindDescriptorSet(layout, res.CamerasDescriptor, 6);

		// Inputs
		shadingConstant.AlbedoTextureID = res.GetBindlessTextureID(GBufferAlbedo->GetID());
		shadingConstant.DepthTextureID = res.GetBindlessTextureID(GBufferDepth->GetID());
		shadingConstant.NormalTextureID = res.GetBindlessTextureID(GBufferNormal->GetID());
		shadingConstant.MaterialTextureID = res.GetBindlessTextureID(GBufferMaterial->GetID());

		// Camera
		shadingConstant.CameraID = ctx.cameraID;

		// Light
		shadingConstant.LightCount = res.LightCount;
		for (int i = 0; i < CSM_AMOUNT; i++)
		{
			shadingConstant.CascadeSplits[i] = LightComponent::mCascadeSplitDepth[i];
		}
	});
	shadingPass.SetRender([&](PassRenderContext& ctx) {
		auto& cmd = ctx.commandBuffer;
		cmd.PushConstants(ctx.renderPass->PipelineLayout, sizeof(ShadingConstant), &shadingConstant);
		
		// Draw full screen quad
		auto& quadMesh = VkSceneRenderer::QuadMesh;
		cmd.BindDescriptorSet(ctx.renderPass->PipelineLayout, quadMesh->GetDescriptorSet(), 1);
		cmd.BindIndexBuffer(quadMesh->GetIndexBuffer()->GetBuffer());
		cmd.DrawIndexed(6);
	});

	ShadowPipeline.Build();
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
