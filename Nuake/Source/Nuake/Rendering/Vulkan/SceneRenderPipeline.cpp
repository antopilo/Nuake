#include "SceneRenderPipeline.h"

#include "Nuake/Rendering/Textures/Material.h"
#include "Nuake/Rendering/Vulkan/VkShaderManager.h"
#include "Nuake/Rendering/Vulkan/VkResources.h"

#include "Nuake/Scene/Scene.h"
#include "Nuake/Scene/Entities/Entity.h"
#include "Nuake/Scene/Components/TransformComponent.h"
#include "Nuake/Scene/Components/ModelComponent.h"

#include "Engine.h"
#include "Nuake/Resource/Project.h"

#include "VulkanAllocator.h"

#include <Tracy.hpp>
#include "DebugCmd.h"
#include <random>

using namespace Nuake;

RenderPipeline ShadowRenderPipeline::ShadowPipeline;

ShadowRenderPipeline::ShadowRenderPipeline()
{
	VkShaderManager& shaderMgr = VkShaderManager::Get();

	ShadowPipeline = RenderPipeline();
	auto& shadowPass = ShadowPipeline.AddPass("Shadow");
	shadowPass.AddAttachment("Depth", ImageFormat::D32F, ImageUsage::Depth, true);
	shadowPass.SetDepthTest(true);
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

std::vector<Vector3> SceneRenderPipeline::ssaoKernelSamples;
Ref<VulkanImage> SceneRenderPipeline::ssaoNoiseTexture;

SceneRenderPipeline::SceneRenderPipeline()
{
	static bool initKernels = false;
	if (!initKernels)
	{
		std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
		std::default_random_engine generator(1337);

		auto lerp = [](float a, float b, float f) -> float
		{
			return a + f * (b - a);
		};

		// Generate kernal samples
		ssaoKernelSamples.reserve(64);
		for (unsigned int i = 0; i < 64; ++i)
		{
			Vector3 sample(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) 
			);
			 
			sample = glm::normalize(sample);
			sample *= randomFloats(generator); 


			float scale = (float)i / 64.0f; 
			scale = lerp(0.1f, 1.0f, scale * scale);
			sample *= scale;
			ssaoKernelSamples.push_back(sample);      
		}  

		auto& resources = GPUResources::Get();
		// Push kernel into allocated buffer
		const size_t bufferSize = sizeof(Vector3) * 64;
		Ref<AllocatedBuffer> buffer = resources.CreateBuffer(bufferSize, BufferUsage::STORAGE_BUFFER | BufferUsage::TRANSFER_DST, MemoryUsage::GPU_ONLY, "ssaoKernels");

		AllocatedBuffer staging = AllocatedBuffer(bufferSize, BufferUsage::TRANSFER_SRC, MemoryUsage::CPU_ONLY);

		void* mappedData;
		vmaMapMemory(VulkanAllocator::Get().GetAllocator(), staging.GetAllocation(), &mappedData);
		memcpy(mappedData, ssaoKernelSamples.data(), buffer->GetSize());
		VkRenderer::Get().ImmediateSubmit([&](VkCommandBuffer cmd)  
		{
			VkBufferCopy copy{ 0 };
			copy.dstOffset = 0;
			copy.srcOffset = 0;
			copy.size = buffer->GetSize();

			vkCmdCopyBuffer(cmd, staging.GetBuffer(), buffer->GetBuffer(), 1, &copy);
		});
		vmaUnmapMemory(VulkanAllocator::Get().GetAllocator(), staging.GetAllocation());

		VkDescriptorBufferInfo transformBufferInfo{};
		transformBufferInfo.buffer = buffer->GetBuffer();
		transformBufferInfo.offset = 0;
		transformBufferInfo.range = VK_WHOLE_SIZE;

		VkWriteDescriptorSet bufferWriteModel = {};
		bufferWriteModel.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		bufferWriteModel.pNext = nullptr;
		bufferWriteModel.dstBinding = 0;
		bufferWriteModel.dstSet = resources.SSAOKernelDescriptor;
		bufferWriteModel.descriptorCount = 1;
		bufferWriteModel.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		bufferWriteModel.pBufferInfo = &transformBufferInfo;
		vkUpdateDescriptorSets(VkRenderer::Get().GetDevice(), 1, &bufferWriteModel, 0, nullptr);    
		   
		// Generate noise texture
		std::vector<Vector2> noiseData;    
		for (unsigned int i = 0; i < 16; i++)   
		{    
			Vector2 noise(
				randomFloats(generator),
				randomFloats(generator)      
			);
			noiseData.push_back(noise);       
		}
		 
		ssaoNoiseTexture = CreateRef<VulkanImage>(noiseData.data(), ImageFormat::RG32F, Vector2{4, 4});
		resources.AddTexture(ssaoNoiseTexture);  

		initKernels = true;
	}

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

	GBufferEntityID = CreateRef<VulkanImage>(ImageFormat::RGBA32F, defaultSize);
	GBufferEntityID->SetDebugName("GBufferEntityID");

	OutlineOutput = CreateRef<VulkanImage>(ImageFormat::RGBA8, defaultSize);
	OutlineOutput->SetDebugName("OutlineOutput");

	LineOutput = CreateRef<VulkanImage>(ImageFormat::RGBA8, defaultSize);
	LineOutput->SetDebugName("OutlineOutput");

	LineCombineOutput = CreateRef<VulkanImage>(ImageFormat::RGBA8, defaultSize);
	LineCombineOutput->SetDebugName("OutlineCombineOutput");

	SSAOOutput = CreateRef<VulkanImage>(ImageFormat::RGBA8, defaultSize);
	SSAOOutput->SetDebugName("SSAOOutput");

	SSAOBlurOutput = CreateRef<VulkanImage>(ImageFormat::RGBA8, defaultSize);
	SSAOBlurOutput->SetDebugName("SSAOBlurOutput");

	GizmoOutput = CreateRef<VulkanImage>(ImageFormat::RGBA8, defaultSize);
	GizmoOutput->SetDebugName("GizmoOutput");

	GizmoCombineOutput = CreateRef<VulkanImage>(ImageFormat::RGBA8, defaultSize);
	GizmoCombineOutput->SetDebugName("GizmoCombineOutput");

	// Setup bloom targets
	BloomOutput = CreateRef<VulkanImage>(ImageFormat::RGBA16F, defaultSize);
	BloomThreshold = CreateRef<VulkanImage>(ImageFormat::RGBA16F, defaultSize);

	RecreatePipeline();
}

SceneRenderPipeline::~SceneRenderPipeline()
{
	auto& res = GPUResources::Get();
	res.RemoveTexture(GBufferAlbedo);
	res.RemoveTexture(GBufferNormal);
	res.RemoveTexture(GBufferMaterial);
	res.RemoveTexture(GBufferDepth);
	res.RemoveTexture(TonemappedOutput);
	res.RemoveTexture(GBufferEntityID);
	res.RemoveTexture(OutlineOutput);
	res.RemoveTexture(LineOutput);
	res.RemoveTexture(LineCombineOutput);
	res.RemoveTexture(SSAOOutput);
	res.RemoveTexture(SSAOBlurOutput);
	res.RemoveTexture(GizmoOutput);
	res.RemoveTexture(GizmoCombineOutput);
	res.RemoveTexture(BloomOutput);
	res.RemoveTexture(BloomThreshold);
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

	GizmoOutput = ResizeImage(ctx, GizmoOutput, ctx.resolution);
	GizmoCombineOutput = ResizeImage(ctx, GizmoCombineOutput, ctx.resolution);

	LineOutput = ResizeImage(ctx, LineOutput, ctx.resolution);
	LineCombineOutput = ResizeImage(ctx, LineCombineOutput, ctx.resolution);

	SSAOOutput = ResizeImage(ctx, SSAOOutput, ctx.resolution);
	SSAOBlurOutput = ResizeImage(ctx, SSAOBlurOutput, ctx.resolution);

	OutlineOutput = ResizeImage(ctx, OutlineOutput, ctx.resolution);

	Color clearColor = ctx.scene->GetEnvironment()->AmbientColor;
	ctx.clearColor = clearColor;

	PipelineAttachments pipelineInputs
	{
		{ GBufferAlbedo, GBufferDepth, GBufferNormal, GBufferMaterial, GBufferEntityID },	// GBuffer
		{ SSAOOutput },
		{ SSAOBlurOutput },
		{ ShadingOutput },													// Shading
		{ TonemappedOutput },												// Tonemap
		{ GizmoOutput, GBufferEntityID, GBufferDepth }, // Reusing depth from gBuffer
		{ GizmoCombineOutput },
		{ OutlineOutput },
		{ LineOutput, GBufferDepth },
		{ LineCombineOutput },
	};

	GBufferPipeline.Execute(ctx, pipelineInputs);

	// Mouse Picking requests
	if (!mousePickingRequests.empty())
	{
		for (auto& request : mousePickingRequests)
		{
			size_t bufferSize = sizeof(Vector4);
			Ref<AllocatedBuffer> stagingBuffer = CreateRef<AllocatedBuffer>(bufferSize, BufferUsage::TRANSFER_DST, MemoryUsage::GPU_TO_CPU);

			// Flip Y
			request.mousePosition.y = GBufferEntityID->GetSize().y - request.mousePosition.y;

			request.mousePosition = glm::clamp(request.mousePosition, { 0, 0 }, GBufferEntityID->GetSize() - Vector2{1, 1});

			VkRenderer::Get().ImmediateSubmit([&](VkCommandBuffer cmd) 
			{
				Cmd command(cmd);
				command.TransitionImageLayout(GBufferEntityID, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
				command.CopyImageToBuffer(GBufferEntityID, stagingBuffer, request.mousePosition);
				command.TransitionImageLayout(GBufferEntityID, VkImageLayout::VK_IMAGE_LAYOUT_GENERAL);
			});

			void* mappedData;
			vmaMapMemory(VulkanAllocator::Get().GetAllocator(), stagingBuffer->GetAllocation(), &mappedData);
			Vector4 entityID = *reinterpret_cast<Vector4*>(mappedData); 
			vmaUnmapMemory(VulkanAllocator::Get().GetAllocator(), stagingBuffer->GetAllocation());

			request.callback(static_cast<int>(entityID.r));
		}

		mousePickingRequests.clear();
	}
}

void SceneRenderPipeline::RecreatePipeline()
{
	VkShaderManager& shaderMgr = VkShaderManager::Get();

	GBufferPipeline = RenderPipeline();
	auto& gBufferPass = GBufferPipeline.AddPass("GBuffer");
	gBufferPass.SetShaders(shaderMgr.GetShader("basic_vert"), shaderMgr.GetShader("basic_frag"));
	gBufferPass.SetDepthTest(true);
	gBufferPass.AddAttachment("Albedo", GBufferAlbedo->GetFormat());
	gBufferPass.AddAttachment("Normal", GBufferNormal->GetFormat());
	gBufferPass.AddAttachment("Material", GBufferMaterial->GetFormat());
	gBufferPass.AddAttachment("EntityID", GBufferEntityID->GetFormat());
	gBufferPass.AddAttachment("Depth", GBufferDepth->GetFormat(), ImageUsage::Depth);
	gBufferPass.SetPushConstant<GBufferConstant>(gbufferConstant);
	gBufferPass.SetPreRender([&](PassRenderContext& ctx) 
	{
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
	gBufferPass.SetRender([&](PassRenderContext& ctx) 
	{
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

	auto& ssaoPass = GBufferPipeline.AddPass("SSAOPass");
	ssaoPass.SetShaders(shaderMgr.GetShader("ssao_vert"), shaderMgr.GetShader("ssao_frag"));
	ssaoPass.SetPushConstant<SSAOConstant>(ssaoConstant);
	ssaoPass.AddAttachment("SSAOOutput", SSAOOutput->GetFormat());
	ssaoPass.AddInput("Depth");
	ssaoPass.AddInput("Normal");
	ssaoPass.SetPreRender([&](PassRenderContext& ctx)
	{
		Cmd& cmd = ctx.commandBuffer;
		auto& layout = ctx.renderPass->PipelineLayout;
		auto& res = GPUResources::Get();
		cmd.BindDescriptorSet(layout, res.ModelDescriptor, 0);
		cmd.BindDescriptorSet(layout, res.SamplerDescriptor, 2);
		cmd.BindDescriptorSet(layout, res.MaterialDescriptor, 3);
		cmd.BindDescriptorSet(layout, res.TexturesDescriptor, 4);
		cmd.BindDescriptorSet(layout, res.LightsDescriptor, 5);
		cmd.BindDescriptorSet(layout, res.CamerasDescriptor, 6);

		// Bind noise kernel
		cmd.BindDescriptorSet(layout, res.SSAOKernelDescriptor, 7);

		// Use noise texture 
		ssaoConstant.noiseTextureID = res.GetBindlessTextureID(ssaoNoiseTexture->GetID());
		ssaoConstant.normalTextureID = res.GetBindlessTextureID(GBufferNormal->GetID());
		ssaoConstant.depthTextureID = res.GetBindlessTextureID(GBufferDepth->GetID());
		ssaoConstant.camViewID = ctx.cameraID;
		ssaoConstant.radius = ctx.scene->GetEnvironment()->mSSAO->Radius;
		ssaoConstant.bias = ctx.scene->GetEnvironment()->mSSAO->Bias;
		ssaoConstant.noiseScale = ctx.resolution / 4.0f;
		ssaoConstant.power = ctx.scene->GetEnvironment()->mSSAO->Strength;

		cmd.PushConstants(layout, sizeof(SSAOConstant), &ssaoConstant);
	});
	ssaoPass.SetRender([&](PassRenderContext& ctx)
	{
		Cmd& cmd = ctx.commandBuffer;
		auto& layout = ctx.renderPass->PipelineLayout;
		auto& quadMesh = VkSceneRenderer::QuadMesh;
		cmd.BindDescriptorSet(ctx.renderPass->PipelineLayout, quadMesh->GetDescriptorSet(), 1);
		cmd.BindIndexBuffer(quadMesh->GetIndexBuffer()->GetBuffer());
		cmd.DrawIndexed(6);
	});

	auto& ssaoBlurPass = GBufferPipeline.AddPass("SSAOBlurPass");
	ssaoBlurPass.SetShaders(shaderMgr.GetShader("blur_vert"), shaderMgr.GetShader("blur_frag"));
	ssaoBlurPass.SetPushConstant<BlurConstant>(blurConstant);
	ssaoBlurPass.AddAttachment("SSAOBlurOutput", SSAOBlurOutput->GetFormat());
	ssaoBlurPass.AddInput("Input");
	ssaoBlurPass.SetPreRender([&](PassRenderContext& ctx) 
	{
		Cmd& cmd = ctx.commandBuffer;
		auto& layout = ctx.renderPass->PipelineLayout;
		auto& res = GPUResources::Get();
		cmd.BindDescriptorSet(layout, res.ModelDescriptor, 0);
		cmd.BindDescriptorSet(layout, res.SamplerDescriptor, 2);
		cmd.BindDescriptorSet(layout, res.MaterialDescriptor, 3);
		cmd.BindDescriptorSet(layout, res.TexturesDescriptor, 4);
		cmd.BindDescriptorSet(layout, res.LightsDescriptor, 5);
		cmd.BindDescriptorSet(layout, res.CamerasDescriptor, 6);
	
		blurConstant.sourceTextureID = res.GetBindlessTextureID(SSAOOutput->GetID());
		blurConstant.sourceSize = SSAOOutput->GetSize();

		cmd.PushConstants(layout, sizeof(BlurConstant), &blurConstant);
	});
	ssaoBlurPass.SetRender([&](PassRenderContext& ctx) 
	{
		Cmd& cmd = ctx.commandBuffer;
		auto& layout = ctx.renderPass->PipelineLayout;
		auto& quadMesh = VkSceneRenderer::QuadMesh;
		cmd.BindDescriptorSet(ctx.renderPass->PipelineLayout, quadMesh->GetDescriptorSet(), 1);
		cmd.BindIndexBuffer(quadMesh->GetIndexBuffer()->GetBuffer());
		cmd.DrawIndexed(6);
	});

	//auto& skyPass = GBufferPipeline.AddPass("SkyPass");
	//skyPass.SetShaders(shaderMgr.GetShader("sky_vert"), shaderMgr.GetShader("sky_frag"));
	//skyPass.SetPushConstant<SkyConstant>(skyConstant);
	//skyPass.AddAttachment("SkyOutput", ShadingOutput->GetFormat());
	//skyPass.SetDepthTest(false);
	//skyPass.SetPreRender([&](PassRenderContext& ctx)
	//{
	//	Cmd& cmd = ctx.commandBuffer;
	//	auto& layout = ctx.renderPass->PipelineLayout;
	//	auto& res = GPUResources::Get();
	//	cmd.BindDescriptorSet(layout, res.ModelDescriptor, 0);
	//	cmd.BindDescriptorSet(layout, res.SamplerDescriptor, 2);
	//	cmd.BindDescriptorSet(layout, res.MaterialDescriptor, 3);
	//	cmd.BindDescriptorSet(layout, res.TexturesDescriptor, 4);
	//	cmd.BindDescriptorSet(layout, res.LightsDescriptor, 5);
	//	cmd.BindDescriptorSet(layout, res.CamerasDescriptor, 6);
	//
	//	skyConstant.cameraID = ctx.cameraID;
	//	auto camera = res.GetCamera(ctx.cameraID);
	//	
	//
	//
	//	skyConstant.ambientColor = ctx.scene->GetEnvironment()->AmbientColor;
	//	skyConstant.ambientTerm = ctx.scene->GetEnvironment()->AmbientTerm;
	//
	//	cmd.PushConstants(ctx.renderPass->PipelineLayout, sizeof(SkyConstant), &skyConstant);
	//});
	//skyPass.SetRender([&](PassRenderContext& ctx)
	//	{
	//		Cmd& cmd = ctx.commandBuffer;
	//		auto& layout = ctx.renderPass->PipelineLayout;
	//		auto& quadMesh = VkSceneRenderer::QuadMesh;
	//		cmd.BindDescriptorSet(ctx.renderPass->PipelineLayout, quadMesh->GetDescriptorSet(), 1);
	//		cmd.BindIndexBuffer(quadMesh->GetIndexBuffer()->GetBuffer());
	//		cmd.DrawIndexed(6);
	//	});
	// skyPass.SetShaders("shaderMgr")

	auto& shadingPass = GBufferPipeline.AddPass("Shading");
	shadingPass.SetShaders(shaderMgr.GetShader("shading_vert"), shaderMgr.GetShader("shading_frag"));
	shadingPass.SetPushConstant<ShadingConstant>(shadingConstant);
	shadingPass.AddAttachment("ShadingOutput", ShadingOutput->GetFormat());
	shadingPass.SetDepthTest(false);
	shadingPass.AddInput("Albedo");
	shadingPass.AddInput("Normal");
	shadingPass.AddInput("Depth");
	shadingPass.AddInput("Material");
	shadingPass.SetPreRender([&](PassRenderContext& ctx) 
	{
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
		shadingConstant.SSAOTextureID = res.GetBindlessTextureID(SSAOBlurOutput->GetID());

		// Camera
		shadingConstant.CameraID = ctx.cameraID;

		// Light
		shadingConstant.LightCount = res.LightCount;
		for (int i = 0; i < CSM_AMOUNT; i++)
		{
			shadingConstant.CascadeSplits[i] = LightComponent::mCascadeSplitDepth[i];
		}
	});
	shadingPass.SetRender([&](PassRenderContext& ctx) 
	{
		auto& cmd = ctx.commandBuffer;
		cmd.PushConstants(ctx.renderPass->PipelineLayout, sizeof(ShadingConstant), &shadingConstant);

		ctx.renderPass->SetClearColor(ctx.scene->GetEnvironment()->AmbientColor);

		// Draw full screen quad
		auto& quadMesh = VkSceneRenderer::QuadMesh;
		cmd.BindDescriptorSet(ctx.renderPass->PipelineLayout, quadMesh->GetDescriptorSet(), 1);
		cmd.BindIndexBuffer(quadMesh->GetIndexBuffer()->GetBuffer());
		cmd.DrawIndexed(6);
	});

	auto& tonemapPass = GBufferPipeline.AddPass("Tonemap");
	tonemapPass.SetShaders(shaderMgr.GetShader("tonemap_vert"), shaderMgr.GetShader("tonemap_frag"));
	tonemapPass.SetPushConstant(tonemapConstant);
	tonemapPass.AddAttachment("TonemapOutput", TonemappedOutput->GetFormat());
	tonemapPass.SetDepthTest(false);
	tonemapPass.AddInput("ShadingOutput");
	tonemapPass.SetPreRender([&](PassRenderContext& ctx) 
	{
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

		ctx.renderPass->SetClearColor(ctx.scene->GetEnvironment()->AmbientColor);

		auto& quadMesh = VkSceneRenderer::QuadMesh;
		cmd.BindDescriptorSet(ctx.renderPass->PipelineLayout, quadMesh->GetDescriptorSet(), 1);
		cmd.BindIndexBuffer(quadMesh->GetIndexBuffer()->GetBuffer());
		cmd.DrawIndexed(6);
	});

	auto& gizmoPass = GBufferPipeline.AddPass("Gizmo");
	gizmoPass.SetShaders(shaderMgr.GetShader("gizmo_vert"), shaderMgr.GetShader("gizmo_frag"));
	gizmoPass.SetPushConstant<DebugConstant>(debugConstant);
	gizmoPass.AddInput("Depth");
	gizmoPass.AddAttachment("GizmoOutput", GizmoOutput->GetFormat());
	gizmoPass.AddAttachment("GizmoEntityID", GBufferEntityID->GetFormat(), ImageUsage::Default, false);
	gizmoPass.AddAttachment("GizmoDepth", GBufferDepth->GetFormat(), ImageUsage::Depth, false);
	gizmoPass.SetDepthTest(true);
	gizmoPass.SetPreRender([&](PassRenderContext& ctx)
	{
		Cmd& cmd = ctx.commandBuffer;
		auto& layout = ctx.renderPass->PipelineLayout;
		auto& res = GPUResources::Get();

		cmd.BindDescriptorSet(layout, res.ModelDescriptor, 0);
		cmd.BindDescriptorSet(layout, res.SamplerDescriptor, 2);
		cmd.BindDescriptorSet(layout, res.MaterialDescriptor, 3);
		cmd.BindDescriptorSet(layout, res.TexturesDescriptor, 4);
		cmd.BindDescriptorSet(layout, res.LightsDescriptor, 5);
		cmd.BindDescriptorSet(layout, res.CamerasDescriptor, 6);
	});
	gizmoPass.SetRender([&](PassRenderContext& ctx)
	{
		auto& cmd = ctx.commandBuffer;
		DebugCmd debugCmd = DebugCmd(cmd, ctx);
		OnDebugDraw().Broadcast(debugCmd);
	});

	auto& gizmoCombinePass = GBufferPipeline.AddPass("GizmoCombine");
	gizmoCombinePass.SetPushConstant(copyConstant);
	gizmoCombinePass.SetShaders(shaderMgr.GetShader("copy_vert"), shaderMgr.GetShader("copy_frag"));
	gizmoCombinePass.AddAttachment("GizmoCombineOutput", GizmoCombineOutput->GetFormat());
	gizmoCombinePass.AddInput("GizmoCombineOutput");
	gizmoCombinePass.SetPreRender([&](PassRenderContext& ctx)
	{
		Cmd& cmd = ctx.commandBuffer;
		auto& layout = ctx.renderPass->PipelineLayout;
		auto& res = GPUResources::Get();

		cmd.BindDescriptorSet(layout, res.ModelDescriptor, 0);
		cmd.BindDescriptorSet(layout, res.SamplerDescriptor, 2);
		cmd.BindDescriptorSet(layout, res.MaterialDescriptor, 3);
		cmd.BindDescriptorSet(layout, res.TexturesDescriptor, 4);
		cmd.BindDescriptorSet(layout, res.LightsDescriptor, 5);
		cmd.BindDescriptorSet(layout, res.CamerasDescriptor, 6);
	});
	gizmoCombinePass.SetRender([&](PassRenderContext& ctx)
	{
		auto& cmd = ctx.commandBuffer;

		copyConstant.SourceTextureID = GPUResources::Get().GetBindlessTextureID(GizmoOutput->GetID());
		copyConstant.Source2TextureID = GPUResources::Get().GetBindlessTextureID(TonemappedOutput->GetID());
		cmd.PushConstants(ctx.renderPass->PipelineLayout, sizeof(copyConstant), &copyConstant);

		auto& quadMesh = VkSceneRenderer::QuadMesh;
		cmd.BindDescriptorSet(ctx.renderPass->PipelineLayout, quadMesh->GetDescriptorSet(), 1);
		cmd.BindIndexBuffer(quadMesh->GetIndexBuffer()->GetBuffer());
		cmd.DrawIndexed(6);
	});

	auto& outlinePass = GBufferPipeline.AddPass("Outline");
	outlinePass.SetShaders(shaderMgr.GetShader("outline_vert"), shaderMgr.GetShader("outline_frag"));
	outlinePass.SetPushConstant<OutlineConstant>(outlineConstant);
	outlinePass.AddAttachment("GizmoCombineOutput", ImageFormat::RGBA8);
	outlinePass.SetDepthTest(false);
	outlinePass.AddInput("ShadingOutput");
	outlinePass.AddInput("EntityID");
	outlinePass.SetPreRender([&](PassRenderContext& ctx) 
	{
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
		outlineConstant.SourceTextureID = GPUResources::Get().GetBindlessTextureID(GizmoCombineOutput->GetID());
		outlineConstant.EntityIDTextureID = GPUResources::Get().GetBindlessTextureID(GBufferEntityID->GetID());
		outlineConstant.DepthTextureID = GPUResources::Get().GetBindlessTextureID(GBufferDepth->GetID());
		outlineConstant.SelectedEntityID = ctx.selectedEntity;
		outlineConstant.Color = Engine::GetProject()->Settings.PrimaryColor;
		outlineConstant.Thickness = Engine::GetProject()->Settings.OutlineRadius;

		auto& cmd = ctx.commandBuffer;
		cmd.PushConstants(ctx.renderPass->PipelineLayout, sizeof(OutlineConstant), &outlineConstant);

		// Draw full screen quad
		auto& quadMesh = VkSceneRenderer::QuadMesh;
		cmd.BindDescriptorSet(ctx.renderPass->PipelineLayout, quadMesh->GetDescriptorSet(), 1);
		cmd.BindIndexBuffer(quadMesh->GetIndexBuffer()->GetBuffer());
		cmd.DrawIndexed(6);
	});

	RenderPass& linePass = GBufferPipeline.AddPass("Line");
	linePass.SetIsLinePass(true);
	linePass.SetTopology(PolygonTopology::LINE_LIST);
	linePass.SetShaders(shaderMgr.GetShader("line_vert"), shaderMgr.GetShader("line_frag"));
	linePass.AddAttachment("LineOutput", LineOutput->GetFormat());
	linePass.AddAttachment("LineDepth", GBufferDepth->GetFormat(), ImageUsage::Depth, false);
	linePass.SetPushConstant(lineConstant);
	linePass.SetDepthTest(true);
	linePass.SetPreRender([&](PassRenderContext& ctx)
	{
		Cmd& cmd = ctx.commandBuffer;
		auto& layout = ctx.renderPass->PipelineLayout;
		auto& res = GPUResources::Get();

		cmd.BindDescriptorSet(layout, res.ModelDescriptor, 0);
		cmd.BindDescriptorSet(layout, res.SamplerDescriptor, 2);
		cmd.BindDescriptorSet(layout, res.MaterialDescriptor, 3);
		cmd.BindDescriptorSet(layout, res.TexturesDescriptor, 4);
		cmd.BindDescriptorSet(layout, res.LightsDescriptor, 5);
		cmd.BindDescriptorSet(layout, res.CamerasDescriptor, 6);
	});
	linePass.SetRender([&](PassRenderContext& ctx)
	{
		auto& cmd = ctx.commandBuffer;
		DebugLineCmd debugCmd = DebugLineCmd(cmd, ctx);
		OnLineDraw().Broadcast(debugCmd);
	});

	auto& lineCombinePass = GBufferPipeline.AddPass("LineCombine");
	lineCombinePass.SetPushConstant(copyConstant);
	lineCombinePass.SetShaders(shaderMgr.GetShader("copy_vert"), shaderMgr.GetShader("copy_frag"));
	lineCombinePass.AddAttachment("LineCombineOutput", LineCombineOutput->GetFormat());
	lineCombinePass.AddInput("lineCombinePassOutput");
	lineCombinePass.SetPreRender([&](PassRenderContext& ctx)
		{
			Cmd& cmd = ctx.commandBuffer;
			auto& layout = ctx.renderPass->PipelineLayout;
			auto& res = GPUResources::Get();

			cmd.BindDescriptorSet(layout, res.ModelDescriptor, 0);
			cmd.BindDescriptorSet(layout, res.SamplerDescriptor, 2);
			cmd.BindDescriptorSet(layout, res.MaterialDescriptor, 3);
			cmd.BindDescriptorSet(layout, res.TexturesDescriptor, 4);
			cmd.BindDescriptorSet(layout, res.LightsDescriptor, 5);
			cmd.BindDescriptorSet(layout, res.CamerasDescriptor, 6);
		});
	lineCombinePass.SetRender([&](PassRenderContext& ctx)
	{
		auto& cmd = ctx.commandBuffer;

		copyConstant.SourceTextureID = GPUResources::Get().GetBindlessTextureID(LineOutput->GetID());
		copyConstant.Source2TextureID = GPUResources::Get().GetBindlessTextureID(OutlineOutput->GetID());
		cmd.PushConstants(ctx.renderPass->PipelineLayout, sizeof(copyConstant), &copyConstant);

		auto& quadMesh = VkSceneRenderer::QuadMesh;
		cmd.BindDescriptorSet(ctx.renderPass->PipelineLayout, quadMesh->GetDescriptorSet(), 1);
		cmd.BindIndexBuffer(quadMesh->GetIndexBuffer()->GetBuffer());
		cmd.DrawIndexed(6);
	});


	GBufferPipeline.Build();
}

void SceneRenderPipeline::MousePick(const Vector2& coord, MousePickingCb mousePickingCb)
{
	assert(coord.x > 0 && coord.y > 0 && "Mouse coords out of bounds");

	mousePickingRequests.push_back({ coord, std::move(mousePickingCb) });
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
