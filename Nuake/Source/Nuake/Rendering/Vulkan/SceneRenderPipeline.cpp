#include "SceneRenderPipeline.h"

#include "Nuake/Rendering/Textures/Material.h"
#include "Nuake/Rendering/Vulkan/VkShaderManager.h"
#include "Nuake/Rendering/Vulkan/VkResources.h"

#include "Nuake/Scene/Scene.h"
#include "Nuake/Scene/Entities/Entity.h"
#include "Nuake/Scene/Components/TransformComponent.h"
#include "Nuake/Scene/Components/ModelComponent.h"

#include <Tracy.hpp>
#include "DebugCmd.h"

using namespace Nuake;

RenderPipeline SceneRenderPipeline::GBufferPipeline;
RenderPipeline ShadowRenderPipeline::ShadowPipeline;

ShadowRenderPipeline::ShadowRenderPipeline()
{
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
			if (!mesh.ModelResource.Get<Model>() || !visibility.Visible)
			{
				continue;
			}

			const int entityId = Entity((entt::entity)e, scene.get()).GetID();
			for (auto& m : mesh.ModelResource.Get<Model>()->GetMeshes())
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

	ShadowPipeline.Build();
}

void ShadowRenderPipeline::Render(PassRenderContext& ctx, Ref<VulkanImage> output)
{
	PipelineAttachments pipelineInputs
	{
		{ output },	
	};

	ShadowPipeline.Execute(ctx, pipelineInputs);
}

SceneRenderPipeline::SceneRenderPipeline()
{
	// Initialize render targets
	const Vector2 defaultSize = { 1, 1 };
	GBufferAlbedo = CreateRef<VulkanImage>(ImageFormat::RGBA16F, defaultSize);
	GBufferAlbedo->SetDebugName("GBufferAlbedo");

	GBufferNormal = CreateRef<VulkanImage>(ImageFormat::RGBA16F, defaultSize);
	GBufferNormal->SetDebugName("GBufferNormal");

	GBufferMaterial = CreateRef<VulkanImage>(ImageFormat::RGBA8, defaultSize);
	GBufferMaterial->SetDebugName("GBufferMaterial");

	GBufferDepth = CreateRef<VulkanImage>(ImageFormat::D32F, defaultSize, ImageUsage::Depth);
	GBufferDepth->SetDebugName("GBufferDepth");

	ShadingOutput = CreateRef<VulkanImage>(ImageFormat::RGBA16F, defaultSize);
	GBufferDepth->SetDebugName("ShadingOutput");

	TonemappedOutput = CreateRef<VulkanImage>(ImageFormat::RGBA8, defaultSize);
	TonemappedOutput->SetDebugName("TonemappedOutput");

	GBufferEntityID = CreateRef<VulkanImage>(ImageFormat::R32F, defaultSize);
	GBufferEntityID->SetDebugName("GBufferEntityID");

	OutlineOutput =CreateRef<VulkanImage>(ImageFormat::RGBA8, defaultSize);
	OutlineOutput->SetDebugName("OutlineOutput");

	// Setup bloom targets
	BloomOutput = CreateRef<VulkanImage>(ImageFormat::RGBA16F, defaultSize);
	BloomThreshold = CreateRef<VulkanImage>(ImageFormat::RGBA16F, defaultSize);

	//Vector2 currentSize;
	//for (int i = 0; i < BloomIteration; i++)
	//{
	//	Ref<VulkanImage> downSample = CreateRef<VulkanImage>(ImageFormat::RGBA16F, defaultSize);
	//	BloomDownSample.push_back(downSample);
	//
	//	m_DownSampleFB.push_back(downSample);
	//	m_UpSampleFB.push_back(upSample);
	//	m_HBlurFB.push_back(hBlur);
	//	m_VBlurFB.push_back(vBlur);
	//}

	// Initialize pipeline
	VkShaderManager& shaderMgr = VkShaderManager::Get();

	GBufferPipeline = RenderPipeline();
	auto& gBufferPass = GBufferPipeline.AddPass("GBuffer");
	gBufferPass.SetShaders(shaderMgr.GetShader("basic_vert"), shaderMgr.GetShader("basic_frag"));
	gBufferPass.AddAttachment("Albedo", GBufferAlbedo->GetFormat());
	gBufferPass.AddAttachment("Normal", GBufferNormal->GetFormat());
	gBufferPass.AddAttachment("Material", GBufferMaterial->GetFormat());
	gBufferPass.AddAttachment("EntityID", GBufferEntityID->GetFormat());
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
			if (!mesh.ModelResource.Get<Model>() || !visibility.Visible)
			{
				continue;
			}

			gbufferConstant.Index = res.ModelMatrixMapping[Entity((entt::entity)e, scene.get()).GetID()];
			
			// Set entity ID
			Entity entity = { (entt::entity)e, scene.get() };
			gbufferConstant.EntityID = static_cast<float>(entity.GetHandle());

			for (auto& m : mesh.ModelResource.Get<Model>()->GetMeshes())
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
	shadingPass.AddAttachment("ShadingOutput", ShadingOutput->GetFormat());
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
		shadingConstant.AmbientTerm = ctx.scene->GetEnvironment()->AmbientTerm;

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

	auto& tonemapPass = GBufferPipeline.AddPass("Tonemap");
	tonemapPass.SetShaders(shaderMgr.GetShader("tonemap_vert"), shaderMgr.GetShader("tonemap_frag"));
	tonemapPass.SetPushConstant<TonemapConstant>(tonemapConstant);
	tonemapPass.AddAttachment("TonemapOutput", TonemappedOutput->GetFormat());
	tonemapPass.SetDepthTest(false);
	tonemapPass.AddInput("ShadingOutput");
	tonemapPass.SetPreRender([&](PassRenderContext& ctx) {
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
		tonemapConstant.Exposure = ctx.scene->GetEnvironment()->Exposure;
		tonemapConstant.SourceTextureID = res.GetBindlessTextureID(ShadingOutput->GetID());
		tonemapConstant.Gamma = ctx.scene->GetEnvironment()->Gamma;
	});
	tonemapPass.SetRender([&](PassRenderContext& ctx)
	{
		auto& cmd = ctx.commandBuffer;
		cmd.PushConstants(ctx.renderPass->PipelineLayout, sizeof(TonemapConstant), &tonemapConstant);

		// Draw full screen quad
		auto& quadMesh = VkSceneRenderer::QuadMesh;
		cmd.BindDescriptorSet(ctx.renderPass->PipelineLayout, quadMesh->GetDescriptorSet(), 1);
		cmd.BindIndexBuffer(quadMesh->GetIndexBuffer()->GetBuffer());
		cmd.DrawIndexed(6);
	});

	auto& outlinePass = GBufferPipeline.AddPass("Outline");
	outlinePass.SetShaders(shaderMgr.GetShader("outline_vert"), shaderMgr.GetShader("outline_frag"));
	outlinePass.SetPushConstant<OutlineConstant>(outlineConstant);
	outlinePass.AddAttachment("OutlineOutput", ImageFormat::RGBA8);
	outlinePass.SetDepthTest(false);
	outlinePass.AddInput("ShadingOutput");
	outlinePass.AddInput("EntityID");
	outlinePass.SetPreRender([&](PassRenderContext& ctx) {
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
	});
	outlinePass.SetRender([&](PassRenderContext& ctx)
	{
		outlineConstant.SourceTextureID = GPUResources::Get().GetBindlessTextureID(ShadingOutput->GetID());
		outlineConstant.EntityIDTextureID = GPUResources::Get().GetBindlessTextureID(GBufferEntityID->GetID());
		outlineConstant.DepthTextureID = GPUResources::Get().GetBindlessTextureID(GBufferDepth->GetID());
		outlineConstant.SelectedEntityID = ctx.selectedEntity;
		outlineConstant.Color = Vector4(1, 0, 0, 1);
		outlineConstant.Thickness = 4.0f;

		auto& cmd = ctx.commandBuffer;
		cmd.PushConstants(ctx.renderPass->PipelineLayout, sizeof(OutlineConstant), &outlineConstant);

		// Draw full screen quad
		auto& quadMesh = VkSceneRenderer::QuadMesh;
		cmd.BindDescriptorSet(ctx.renderPass->PipelineLayout, quadMesh->GetDescriptorSet(), 1);
		cmd.BindIndexBuffer(quadMesh->GetIndexBuffer()->GetBuffer());
		cmd.DrawIndexed(6);
	});

	GBufferPipeline.Build();
}

void SceneRenderPipeline::SetCamera(UUID camera)
{
	CurrentCameraID = camera;
}

void SceneRenderPipeline::Render(PassRenderContext& ctx)
{
	// Resize textures
	GBufferAlbedo = ResizeImage(ctx, GBufferAlbedo, ctx.resolution);
	GBufferDepth = ResizeImage(ctx, GBufferDepth, ctx.resolution);
	GBufferNormal = ResizeImage(ctx, GBufferNormal, ctx.resolution);
	GBufferMaterial = ResizeImage(ctx, GBufferMaterial, ctx.resolution);
	GBufferEntityID = ResizeImage(ctx, GBufferEntityID, ctx.resolution);

	ShadingOutput = ResizeImage(ctx, ShadingOutput, ctx.resolution);
	TonemappedOutput = ResizeImage(ctx, TonemappedOutput, ctx.resolution);

	OutlineOutput = ResizeImage(ctx, OutlineOutput, ctx.resolution);

	PipelineAttachments pipelineInputs
	{
		{ GBufferAlbedo, GBufferDepth, GBufferNormal, GBufferMaterial, GBufferEntityID },	// GBuffer
		{ ShadingOutput },													// Shading
		{ TonemappedOutput },												// Tonemap
		{ OutlineOutput }
	};

	GBufferPipeline.Execute(ctx, pipelineInputs);

	// Debug drawing
	DebugCmd debugCmd = DebugCmd(ctx.commandBuffer);
	OnDebugDraw().Broadcast(debugCmd);
}

Ref<VulkanImage> SceneRenderPipeline::ResizeImage(PassRenderContext& ctx, Ref<VulkanImage> image, const Vector2& size)
{
	if (image->GetSize() == size)
	{
		return image;
	}

	Ref<VulkanImage> newAttachment = std::make_shared<VulkanImage>(image->GetFormat(), size, image->GetUsage());
	newAttachment->SetDebugName(image->GetDebugName().data());

	// Register to resource manager
	GPUResources& gpuResources = GPUResources::Get();
	gpuResources.AddTexture(newAttachment);
	gpuResources.RemoveTexture(image);

	// We might need to do this?
	ctx.commandBuffer.TransitionImageLayout(newAttachment, VK_IMAGE_LAYOUT_GENERAL);
	return newAttachment;
}
