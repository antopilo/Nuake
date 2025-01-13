#include "VulkanSceneRenderer.h"

#include "src/Rendering/Textures/Material.h"

#include "src/Rendering/Vulkan/Pipeline/RenderPipeline.h"
#include "src/Rendering/Vulkan/SceneRenderPipeline.h"
#include "src/Rendering/Vulkan/ShaderCompiler.h"
#include "src/Rendering/Vulkan/VkMesh.h"
#include "src/Rendering/Vulkan/VkResources.h"
#include "src/Rendering/Vulkan/VkShaderManager.h"
#include "src/Rendering/Vulkan/VulkanAllocator.h"
#include "src/Rendering/Vulkan/VulkanCheck.h"
#include "src/Rendering/Vulkan/VulkanInit.h"
#include "src/Rendering/Vulkan/VulkanRenderer.h"

#include "src/Scene/Scene.h"
#include "src/Scene/Entities/Entity.h"
#include "src/Scene/Components/ModelComponent.h"
#include "src/Scene/Components/CameraComponent.h"

#include <Tracy.hpp>

using namespace Nuake;

Ref<VkMesh> VkSceneRenderer::QuadMesh;

void VkSceneRenderer::Init()
{
	LoadShaders();
	SetGBufferSize({ 1280, 720 });
	CreatePipelines();

    const std::vector<Vertex> quadVertices 
	{
		{ Vector3(-1.0f,  1.0f, 1.0f), 0.0f, Vector3(0, 0, 1), 1.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
		{ Vector3(1.0f,  1.0f,  1.0f),  1.0f, Vector3(0, 0, 1), 1.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
		{ Vector3(-1.0f, -1.0f, 1.0f), 0.0f, Vector3(0, 0, 1), 0.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
		{ Vector3(1.0f,  -1.0f, 1.0f), 1.0f, Vector3(0, 0, 1), 0.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
		{ Vector3(-1.0f, -1.0f, 1.0f), 0.0f, Vector3(0, 0, 1), 0.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
		{ Vector3(1.0f,   1.0f, 1.0f), 1.0f, Vector3(0, 0, 1), 1.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) }
    };

    const std::vector<uint32_t> quadIndices
	{
        5, 4, 3, 2, 1, 0
    };

	QuadMesh = CreateRef<VkMesh>(quadVertices, quadIndices);
}

// This will prepare all the data and upload it to the GPU before rendering the scene.
void VkSceneRenderer::BeginScene(RenderContext inContext)
{
	Context.CommandBuffer = inContext.CommandBuffer;
	Context.CurrentScene = inContext.CurrentScene;
	Context.CameraID = inContext.CameraID;

	auto& scene = Context.CurrentScene;
	auto& gpu = GPUResources::Get();

	// CameraView
	{
		// Clear last frame's cameras
		gpu.ClearCameras();

		// Editor camera, maybe strip this out in runtime?
		const auto& camera = scene->m_EditorCamera;
		CameraView cameraView 
		{
			.View = camera->GetTransform(),
			.Projection = camera->GetPerspective(),
			.InverseView = glm::inverse(cameraView.View),
			.InverseProjection = glm::inverse(cameraView.Projection),
			.Position = camera->GetTranslation(),
			.Near = camera->Near,
			.Far = camera->Far,
		};
		gpu.AddCamera(camera->ID, std::move(cameraView));

		// Add scene cameras
		auto view = scene->m_Registry.view<TransformComponent, CameraComponent>();
		for (auto e : view)
		{
			const auto& [transform, cameraComponent] = view.get<TransformComponent, CameraComponent>(e);
			const Ref<Camera> camera = cameraComponent.CameraInstance;

			CameraView cameraView
			{
				.View = camera->GetTransform(),
				.Projection = camera->GetPerspective(),
				.InverseView = glm::inverse(cameraView.View),
				.InverseProjection = glm::inverse(cameraView.Projection),
				.Position = camera->GetTranslation(),
				.Near = camera->Near,
				.Far = camera->Far,
			};
			gpu.AddCamera(camera->ID, std::move(cameraView));
		}
	}

	// CSM Light's view
	{
		auto view = scene->m_Registry.view<TransformComponent, LightComponent>();
		for (auto e : view)
		{
			auto [transform, light] = view.get<TransformComponent, LightComponent>(e);
			for (auto& view : light.m_LightViews)
			{
				CameraView cameraView
				{
					.View = view.View,
					.Projection = view.Proj,
					.InverseView = glm::inverse(view.View),
					.InverseProjection = glm::inverse(view.Proj),
					.Position = transform.GetGlobalTransform()[3],
					.Near = 0,
					.Far = 0,
				};
				gpu.AddCamera(view.CameraID, std::move(cameraView));
			}
		}
	}

	// All transforms & materials`
	{
		uint32_t currentIndex = 0;
		uint32_t currentMaterialIndex = 0;
		std::array<Matrix4, MAX_MODEL_MATRIX> allTransforms;
		std::array<MaterialBufferStruct, MAX_MATERIAL> allMaterials;
		auto view = scene->m_Registry.view<TransformComponent, ModelComponent, VisibilityComponent>();
		for (auto e : view)
		{
			// Check if we've reached the maximum capacity of the array
			if (currentIndex >= MAX_MODEL_MATRIX)
			{
				assert(false && "Max model matrix reached!");
				break;
			}

			auto [transform, mesh, visibility] = view.get<TransformComponent, ModelComponent, VisibilityComponent>(e);
			if (!mesh.ModelResource || !visibility.Visible)
			{
				continue;
			}

			// Upload transforms to GPU resources
			allTransforms[currentIndex] = transform.GetGlobalTransform();
			gpu.ModelMatrixMapping[Entity((entt::entity)e, scene.get()).GetID()] = currentIndex;

			// Upload mesh material to GPU resources
			for (auto& m : mesh.ModelResource->GetMeshes())
			{
				// TODO: Avoid duplicated materials
				if (Ref<Material> material = m->GetMaterial(); material)
				{
					MaterialBufferStruct materialBuffer
					{
						.HasAlbedo = material->HasAlbedo(),
						.AlbedoColor = material->data.m_AlbedoColor,
						.HasNormal = material->HasNormal(),
						.HasMetalness = material->HasMetalness(),
						.HasRoughness = material->HasRoughness(),
						.HasAO = material->HasAO(),
						.MetalnessValue = material->data.u_MetalnessValue,
						.RoughnessValue = material->data.u_RoughnessValue,
						.AoValue = material->data.u_AOValue,
						.AlbedoTextureId = material->HasAlbedo() ? gpu.GetBindlessTextureID(material->AlbedoImage) : 0,
						.NormalTextureId = material->HasNormal() ? gpu.GetBindlessTextureID(material->NormalImage) : 0,
						.MetalnessTextureId = material->HasMetalness() ? gpu.GetBindlessTextureID(material->MetalnessImage) : 0,
						.RoughnessTextureId = material->HasRoughness() ? gpu.GetBindlessTextureID(material->RoughnessImage) : 0,
						.AoTextureId = material->HasAO() ? gpu.GetBindlessTextureID(material->AOImage) : 0,
					};

					// Save bindless mapping index
					allMaterials[currentMaterialIndex] = std::move(materialBuffer);
					gpu.MeshMaterialMapping[m->GetVkMesh()->GetID()] = currentMaterialIndex;
					currentMaterialIndex++;
				}
			}

			currentIndex++;
		}

		gpu.ModelTransforms = ModelData{ allTransforms };
		gpu.MaterialDataContainer = MaterialData{ allMaterials };
	}

	// All lights
	{
		uint32_t lightCount = 0;
		std::array<LightData, MAX_LIGHTS> allLights;
		auto lightView = scene->m_Registry.view<TransformComponent, LightComponent>();
		for (auto e : lightView)
		{
			if (lightCount >= MAX_LIGHTS)
			{
				assert(false && "Max amount of light reached!");
				break;
			}

			auto [transform, lightComp] = lightView.get<TransformComponent, LightComponent>(e);

			// Update light direction with transform, shouldn't be here!
			// TODO: Move to transform system
			lightComp.Direction = transform.GetGlobalRotation() * Vector3(0, 0, -1);

			LightData light
			{
				.Position = Vector3(transform.GetGlobalTransform()[3]),
				.Type = lightComp.Type,
				.Color = Vector4(lightComp.Color * lightComp.Strength, 1.0),
				.Direction = lightComp.Direction,
				.OuterConeAngle = glm::cos(Rad(lightComp.OuterCutoff)),
				.InnerConeAngle = glm::cos(Rad(lightComp.Cutoff)),
				.CastShadow = lightComp.CastShadows,
			};

			for (int i = 0; i < CSM_AMOUNT; i++)
			{
				light.TransformId[i] = gpu.GetBindlessCameraID(lightComp.m_LightViews[i].CameraID);
				light.ShadowMapTextureId[i] = gpu.GetBindlessTextureID(lightComp.LightMapID);
			}

			allLights[lightCount] = std::move(light);

			lightCount++;
		}

		gpu.LightDataContainerArray = LightDataContainer{ allLights };
		gpu.LightCount = lightCount;
	}


	// Copy CSM split depths
	for (int i = 0; i < CSM_AMOUNT; i++)
	{
		//shadingPushConstant.CascadeSplits[i] = LightComponent::mCascadeSplitDepth[i];
	}

	// Update transforms, materials and lights.
	// We need to push lights first to have bindless mapping for CSM
	gpu.UpdateBuffers();

	// Update light CSM
	{
		auto view = scene->m_Registry.view<TransformComponent, LightComponent>();
		for (auto e : view)
		{
			auto [transform, light] = view.get<TransformComponent, LightComponent>(e);
			auto cam = gpu.GetCamera(inContext.CameraID);

			if (light.Type == LightType::Directional)
			{
				light.CalculateViewProjection(cam.View, cam.Projection);
			}
		}
	}

	gpu.RecreateBindlessCameras();

	// Execute light
	PassRenderContext passCtx = { };
	passCtx.scene = inContext.CurrentScene;
	passCtx.commandBuffer = inContext.CommandBuffer;
	passCtx.resolution = Context.Size;
	passCtx.cameraID = GPUResources::Get().GetBindlessCameraID(inContext.CameraID);

	auto view = scene->m_Registry.view<TransformComponent, LightComponent>();
	for (auto e : view)
	{
		auto [transform, light] = view.get<TransformComponent, LightComponent>(e);

		if (light.Type != LightType::Directional)
		{
			continue;
		}
		
		// TODO: Execute shadow pipeline for each light

		passCtx.cameraID = GPUResources::Get().GetBindlessCameraID(light.m_LightViews[0].CameraID);
		//ShadowPipeline.Execute(passCtx);
		
		//light.LightMapID = ShadowPipeline.GetRenderPass("Shadow").GetDepthAttachment().Image->GetID();

		//passCtx.cameraID = GPUResources::Get().GetBindlessCameraID(light.m_LightViews[0].CameraID);
		for (int i = 0; i < CSM_AMOUNT; i++)
		{
			//ShadowPipeline.Execute(passCtx);
		}
	}

	//GBufferPipeline.Execute(passCtx);

	// Set back the camera ID to the actual desired camera.
	passCtx.cameraID = GPUResources::Get().GetBindlessCameraID(inContext.CameraID);
	sceneRenderPipeline->Render(passCtx);
}

void VkSceneRenderer::EndScene()
{
	// Copy final output to DrawImage.
	Ref<VulkanImage> drawImage = VkRenderer::Get().GetDrawImage();
	Ref<VulkanImage> output = sceneRenderPipeline->GetOutput();
	
	Cmd& cmd = Context.CommandBuffer;
	cmd.TransitionImageLayout(output, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	cmd.TransitionImageLayout(drawImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	cmd.CopyImageToImage(output, drawImage);
	cmd.TransitionImageLayout(drawImage, VK_IMAGE_LAYOUT_GENERAL);
	cmd.TransitionImageLayout(output, VK_IMAGE_LAYOUT_GENERAL);
}

void VkSceneRenderer::LoadShaders()
{
	// TODO: load embedded shaders in the future
	VkShaderManager& shaderMgr = VkShaderManager::Get();
	ShaderCompiler& shaderCompiler = ShaderCompiler::Get();
	shaderMgr.AddShader("basic_frag", shaderCompiler.CompileShader("../Resources/Shaders/Vulkan/triangle.frag"));
	shaderMgr.AddShader("basic_vert", shaderCompiler.CompileShader("../Resources/Shaders/Vulkan/triangle.vert"));
	shaderMgr.AddShader("shading_frag", shaderCompiler.CompileShader("../Resources/Shaders/Vulkan/shading.frag"));
	shaderMgr.AddShader("shading_vert", shaderCompiler.CompileShader("../Resources/Shaders/Vulkan/shading.vert"));
	shaderMgr.AddShader("shadow_frag", shaderCompiler.CompileShader("../Resources/Shaders/Vulkan/shadow.frag"));
	shaderMgr.AddShader("shadow_vert", shaderCompiler.CompileShader("../Resources/Shaders/Vulkan/shadow.vert"));
}

void VkSceneRenderer::CreatePipelines()
{
	sceneRenderPipeline = CreateRef<SceneRenderPipeline>();

	//ShadowPipeline = RenderPipeline();
	//auto& shadowPass = ShadowPipeline.AddPass("Shadow");
	//shadowPass.AddAttachment("Depth", ImageFormat::D32F, ImageUsage::Depth);
	//shadowPass.SetShaders(Shaders["shadow_vert"], Shaders["shadow_frag"]);
	//shadowPass.SetPushConstant<ModelPushConstant>(modelPushConstant);
	//shadowPass.SetPreRender([&](PassRenderContext& ctx) {
	//	auto layout = ctx.renderPass->PipelineLayout;
	//	ctx.commandBuffer.BindDescriptorSet(layout, CameraBufferDescriptors, 0);
	//	ctx.commandBuffer.BindDescriptorSet(layout, GPUResources::Get().ModelDescriptor, 1);
	//	ctx.commandBuffer.BindDescriptorSet(layout, SamplerDescriptor, 3);
	//	ctx.commandBuffer.BindDescriptorSet(layout, MaterialBufferDescriptor, 4);
	//	ctx.commandBuffer.BindDescriptorSet(layout, GPUResources::Get().TexturesDescriptor, 5);
	//	ctx.commandBuffer.BindDescriptorSet(layout, LightBufferDescriptor, 6);
	//	ctx.commandBuffer.BindDescriptorSet(layout, GPUResources::Get().CamerasDescriptor, 7);
	//});
	//shadowPass.SetRender([&](PassRenderContext& ctx) {
	//	auto& cmd = ctx.commandBuffer;
	//	auto& scene = ctx.scene;
	//	auto& vk = VkRenderer::Get();
	//
	//	// Draw the scene
	//	{
	//		ZoneScopedN("Render Models");
	//		auto view = scene->m_Registry.view<TransformComponent, ModelComponent, VisibilityComponent>();
	//		for (auto e : view)
	//		{
	//			auto [transform, mesh, visibility] = view.get<TransformComponent, ModelComponent, VisibilityComponent>(e);
	//			if (!mesh.ModelResource || !visibility.Visible)
	//			{
	//				continue;
	//			}
	//
	//			Entity entity = Entity((entt::entity)e, scene.get());
	//			for (auto& m : mesh.ModelResource->GetMeshes())
	//			{
	//				Ref<VkMesh> vkMesh = m->GetVkMesh();
	//				Matrix4 globalTransform = transform.GetGlobalTransform();
	//
	//				auto descSet = vkMesh->GetDescriptorSet();
	//				cmd.BindDescriptorSet(ctx.renderPass->PipelineLayout, descSet, 2);
	//
	//				// Bind texture descriptor set
	//				Ref<Material> material = m->GetMaterial();
	//				Ref<VulkanImage> albedo = GPUResources::Get().GetTexture(material->AlbedoImage);
	//
	//
	//
	//				modelPushConstant.Index = GPUResources::Get().GetBindlessTransformID(entity.GetID());
	//				modelPushConstant.MaterialIndex = GPUResources::Get().GetBindlessMaterialID(entity.GetID());
	//				modelPushConstant.CameraID = ctx.cameraID;
	//
	//				cmd.PushConstants(ctx.renderPass->PipelineLayout, sizeof(ModelPushConstant), &modelPushConstant);
	//				cmd.BindIndexBuffer(vkMesh->GetIndexBuffer()->GetBuffer());
	//				cmd.DrawIndexed(vkMesh->GetIndexBuffer()->GetSize() / sizeof(uint32_t));
	//			}
	//		}
	//	}
	//});
	//ShadowPipeline.Build();
	//
	//GBufferPipeline = RenderPipeline();
	//auto& gBufferPass = GBufferPipeline.AddPass("GBuffer");
	//gBufferPass.SetShaders(Shaders["basic_vert"], Shaders["basic_frag"]);
	//gBufferPass.AddAttachment("Albedo", ImageFormat::RGBA8);
	//gBufferPass.AddAttachment("Normal", ImageFormat::RGBA8);
	//gBufferPass.AddAttachment("Material", ImageFormat::RGBA8);
	//gBufferPass.AddAttachment("Depth", ImageFormat::D32F, ImageUsage::Depth);
	//gBufferPass.SetPushConstant<ModelPushConstant>(modelPushConstant);
	//gBufferPass.SetPreRender([&](PassRenderContext& ctx) {
	//	auto layout = ctx.renderPass->PipelineLayout;
	//	ctx.commandBuffer.BindDescriptorSet(layout, CameraBufferDescriptors, 0);
	//	ctx.commandBuffer.BindDescriptorSet(layout, GPUResources::Get().ModelDescriptor, 1);
	//	ctx.commandBuffer.BindDescriptorSet(layout, SamplerDescriptor, 3);
	//	ctx.commandBuffer.BindDescriptorSet(layout, MaterialBufferDescriptor, 4);
	//	ctx.commandBuffer.BindDescriptorSet(layout, GPUResources::Get().TexturesDescriptor, 5);
	//	ctx.commandBuffer.BindDescriptorSet(layout, LightBufferDescriptor, 6);
	//	ctx.commandBuffer.BindDescriptorSet(layout, GPUResources::Get().CamerasDescriptor, 7);
	//});
	//gBufferPass.SetRender([&](PassRenderContext& ctx){
	//	auto& cmd = ctx.commandBuffer;
	//	auto& scene = ctx.scene;
	//	auto& vk = VkRenderer::Get();
	//
	//	// Draw the scene
	//	{
	//		ZoneScopedN("Render Models");
	//		auto view = scene->m_Registry.view<TransformComponent, ModelComponent, VisibilityComponent>();
	//		for (auto e : view)
	//		{
	//			auto [transform, mesh, visibility] = view.get<TransformComponent, ModelComponent, VisibilityComponent>(e);
	//			if (!mesh.ModelResource || !visibility.Visible)
	//			{
	//				continue;
	//			}
	//
	//			Entity entity = Entity((entt::entity)e, scene.get());
	//			for (auto& m : mesh.ModelResource->GetMeshes())
	//			{
	//				Ref<VkMesh> vkMesh = m->GetVkMesh();
	//				Matrix4 globalTransform = transform.GetGlobalTransform();
	//
	//				auto descSet = vkMesh->GetDescriptorSet();
	//				cmd.BindDescriptorSet(ctx.renderPass->PipelineLayout, descSet, 2);
	//
	//				// Bind texture descriptor set
	//				Ref<Material> material = m->GetMaterial();
	//				Ref<VulkanImage> albedo = GPUResources::Get().GetTexture(material->AlbedoImage);
	//
	//				modelPushConstant.Index = GPUResources::Get().GetBindlessTransformID(entity.GetID());
	//				modelPushConstant.MaterialIndex = GPUResources::Get().GetBindlessMaterialID(entity.GetID());
	//				modelPushConstant.CameraID = ctx.cameraID;
	//				cmd.PushConstants(ctx.renderPass->PipelineLayout, sizeof(ModelPushConstant), &modelPushConstant);
	//
	//				cmd.BindIndexBuffer(vkMesh->GetIndexBuffer()->GetBuffer());
	//				cmd.DrawIndexed(vkMesh->GetIndexBuffer()->GetSize() / sizeof(uint32_t));
	//			}
	//		}
	//	}
	//	
	//});
	//
	//auto& shadingPass = GBufferPipeline.AddPass("Shading");
	//shadingPass.SetShaders(Shaders["shading_vert"], Shaders["shading_frag"]);
	//shadingPass.SetPushConstant<ShadingPushConstant>(shadingPushConstant);
	//shadingPass.AddAttachment("Output", ImageFormat::RGBA8);
	//shadingPass.SetDepthTest(false);
	//shadingPass.AddInput("Albedo");
	//shadingPass.AddInput("Normal");
	//shadingPass.AddInput("Depth");
	//shadingPass.AddInput("Material");
	//shadingPass.SetPreRender([&](PassRenderContext& ctx) {
	//	auto layout = ctx.renderPass->PipelineLayout;
	//	auto cmd = ctx.commandBuffer;
	//	cmd.BindDescriptorSet(layout, CameraBufferDescriptors, 0);
	//	cmd.BindDescriptorSet(layout, GPUResources::Get().ModelDescriptor, 1);
	//	cmd.BindDescriptorSet(layout, SamplerDescriptor, 3);
	//	cmd.BindDescriptorSet(layout, MaterialBufferDescriptor, 4);
	//	cmd.BindDescriptorSet(layout, GPUResources::Get().TexturesDescriptor, 5);
	//	cmd.BindDescriptorSet(layout, LightBufferDescriptor, 6);
	//	cmd.BindDescriptorSet(layout, GPUResources::Get().CamerasDescriptor, 7);
	//
	//	auto& gpu = GPUResources::Get();
	//	auto& gbufferPass = GBufferPipeline.GetRenderPass("GBuffer");
	//	shadingPushConstant.AlbedoTextureID = gpu.GetBindlessTextureID(gbufferPass.GetAttachment("Albedo").Image->GetID());
	//	shadingPushConstant.DepthTextureID = gpu.GetBindlessTextureID(gbufferPass.GetDepthAttachment().Image->GetID());
	//	shadingPushConstant.NormalTextureID = gpu.GetBindlessTextureID(gbufferPass.GetAttachment("Normal").Image->GetID());
	//	shadingPushConstant.MaterialTextureID = gpu.GetBindlessTextureID(gbufferPass.GetAttachment("Material").Image->GetID());
	//	shadingPushConstant.CameraID = ctx.cameraID;
	//});
	//shadingPass.SetRender([](PassRenderContext& ctx) {
	//	auto& cmd = ctx.commandBuffer;
	//	cmd.PushConstants(ctx.renderPass->PipelineLayout, sizeof(ShadingPushConstant), &shadingPushConstant);
	//
	//	// Draw full screen quad
	//	cmd.BindDescriptorSet(ctx.renderPass->PipelineLayout, quadMesh->GetDescriptorSet(), 2);
	//	cmd.BindIndexBuffer(quadMesh->GetIndexBuffer()->GetBuffer());
	//	cmd.DrawIndexed(6);
	//});
	//
	//GBufferPipeline.Build();
	//ShadowPipeline.Build();
}

void VkSceneRenderer::SetGBufferSize(const Vector2& size)
{
	Context.Size = size;
}

void VkSceneRenderer::BuildMatrixBuffer()
{
	// This will scan and build the matrix buffer for the next frame.
	// It will create a mapping between UUID and the corresponding index for the model matrix
	ZoneScopedN("Build Matrix Buffer");
	auto& scene = Context.CurrentScene;
	auto& res = GPUResources::Get();

	
}
