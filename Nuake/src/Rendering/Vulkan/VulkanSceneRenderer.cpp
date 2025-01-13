#include "PipelineBuilder.h"
#include "ShaderCompiler.h"
#include "VulkanCheck.h"
#include "VulkanSceneRenderer.h"

#include "src/Rendering/Vulkan/VulkanAllocator.h"
#include "src/Rendering/Vulkan/VulkanInit.h"
#include "src/Rendering/Vulkan/VulkanRenderer.h"
#include "src/Rendering/Vulkan/VkResources.h"

#include "src/Scene/Scene.h"
#include "src/Scene/Entities/Entity.h"
#include "src/Rendering/Textures/Material.h"


#include <Tracy.hpp>
#include <src/Scene/Components/ModelComponent.h>
#include "Pipeline/RenderPipeline.h"

#include "src/Rendering/Vulkan/DescriptorLayoutBuilder.h"
#include "src/Scene/Components/CameraComponent.h"
#include "src/Rendering/Vulkan/VkShaderManager.h"
#include "src/Rendering/Vulkan/SceneRenderPipeline.h"


using namespace Nuake;

VkSceneRenderer::VkSceneRenderer()
{
}

VkSceneRenderer::~VkSceneRenderer()
{
}

Ref<VkMesh> quadMesh;
void VkSceneRenderer::Init()
{
	// Here we will create the pipeline for rendering a scene
	LoadShaders();
	CreateDescriptors();

	SetGBufferSize({ 1280, 720 });
	CreatePipelines();

    std::vector<Vertex> quadVertices = {
		{ Vector3(-1.0f,  1.0f, 1.0f), 0.0f, Vector3(0, 0, 1), 1.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
		{ Vector3(1.0f,  1.0f,  1.0f),  1.0f, Vector3(0, 0, 1), 1.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
		{ Vector3(-1.0f, -1.0f, 1.0f), 0.0f, Vector3(0, 0, 1), 0.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
		{ Vector3(1.0f,  -1.0f, 1.0f), 1.0f, Vector3(0, 0, 1), 0.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
		{ Vector3(-1.0f, -1.0f, 1.0f), 0.0f, Vector3(0, 0, 1), 0.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
		{ Vector3(1.0f,   1.0f, 1.0f), 1.0f, Vector3(0, 0, 1), 1.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) }
    };

    std::vector<uint32_t> quadIndices = {
        5, 4, 3, 2, 1, 0
    };

	quadMesh = CreateRef<VkMesh>(quadVertices, quadIndices);
}

void VkSceneRenderer::BeginScene(RenderContext inContext)
{
	GPUResources::Get().ClearCameras();

	Context.CommandBuffer = inContext.CommandBuffer;
	Context.CurrentScene = inContext.CurrentScene;
	Context.CameraID = inContext.CameraID;

	// Collect all global transform of things we will render
	auto& cmd = Context.CommandBuffer;
	auto& scene = Context.CurrentScene;
	auto& vk = VkRenderer::Get();

	// Ensure the draw image is valid
	if (!vk.DrawImage) {
		throw std::runtime_error("Draw image is not initialized");
	}

	// Build camera view list
	{
		auto camera = scene->m_EditorCamera;
		CameraView camData{};
		camData.View = camera->GetTransform();
		camData.Projection = camera->GetPerspective();
		camData.InverseView = glm::inverse(camData.View);
		camData.InverseProjection = glm::inverse(camData.Projection);
		camData.Position = camera->GetTranslation();
		camData.Near = camera->Near;
		camData.Far = camera->Far;
		GPUResources::Get().AddCamera(camera->ID, camData);
		GPUResources::Get().AddCamera(camera->ID + 1, camData);

		auto view = scene->m_Registry.view<TransformComponent, CameraComponent>();
		for (auto e : view)
		{
			auto [transform, camera] = view.get<TransformComponent, CameraComponent>(e);
			CameraView camData{};
			camData.View = camera.CameraInstance->GetTransform();
			camData.Projection = camera.CameraInstance->GetPerspective();
			camData.InverseView = glm::inverse(camData.View);
			camData.InverseProjection = glm::inverse(camData.Projection);
			camData.Position = transform.GetGlobalTransform()[3];
			camData.Near = camera.CameraInstance->Near;
			camData.Far = camera.CameraInstance->Far;
			GPUResources::Get().AddCamera(camera.ID, camData);
		}
	}

	// Build light view list

	{
		auto view = scene->m_Registry.view<TransformComponent, LightComponent>();
		for (auto e : view)
		{
			auto [transform, light] = view.get<TransformComponent, LightComponent>(e);

			for (int i = 0; i < CSM_AMOUNT; i++)
			{
				LightView& view = light.m_LightViews[i];
				CameraView viewData{};
				viewData.View = view.View;
				viewData.Projection = view.Proj;
				viewData.InverseView = glm::inverse(view.View);
				viewData.InverseProjection = glm::inverse(view.Proj);
				viewData.Position = transform.GetGlobalTransform()[3];
				viewData.Near = 0;
				viewData.Far = 0;
				GPUResources::Get().AddCamera(view.CameraID, viewData);
			}
		}
	}

	BuildMatrixBuffer();
	UpdateTransformBuffer();

	{
		auto view = scene->m_Registry.view<TransformComponent, LightComponent>();
		for (auto e : view)
		{
			auto [transform, light] = view.get<TransformComponent, LightComponent>(e);
			auto cam = GPUResources::Get().GetCamera(inContext.CameraID);

			if (light.Type == LightType::Directional)
			{
				light.CalculateViewProjection(cam.View, cam.Projection);
			}
		}
	}

	GPUResources::Get().RecreateBindlessCameras();

	// Execute light
	PassRenderContext passCtx = { };
	passCtx.scene = inContext.CurrentScene;
	passCtx.commandBuffer = inContext.CommandBuffer;
	passCtx.resolution = Context.Size;
	
	auto view = scene->m_Registry.view<TransformComponent, LightComponent>();
	for (auto e : view)
	{
		auto [transform, light] = view.get<TransformComponent, LightComponent>(e);

		if (light.Type != LightType::Directional)
		{
			continue;
		}

		//passCtx.cameraID = GPUResources::Get().GetBindlessCameraID(light.m_LightViews[0].CameraID);
		//ShadowPipeline.Execute(passCtx);
		
		//light.LightMapID = ShadowPipeline.GetRenderPass("Shadow").GetDepthAttachment().Image->GetID();

		//passCtx.cameraID = GPUResources::Get().GetBindlessCameraID(light.m_LightViews[0].CameraID);
		for (int i = 0; i < CSM_AMOUNT; i++)
		{
			//ShadowPipeline.Execute(passCtx);
		}
	}


	//passCtx.cameraID = GPUResources::Get().GetBindlessCameraID(Context.CameraID);
	//GBufferPipeline.Execute(passCtx);

	sceneRenderPipeline->Render(passCtx);
}


void VkSceneRenderer::EndScene()
{
	auto& vk = VkRenderer::Get();
	auto& cmd = Context.CommandBuffer;

	//auto& albedo = GBufferPipeline.GetRenderPass("GBuffer").GetAttachment("Albedo");
	//auto& normal = GBufferPipeline.GetRenderPass("GBuffer").GetAttachment("Normal");
	//auto& shading = GBufferPipeline.GetRenderPass("Shading").GetAttachment("Output");
	//auto& shadow = ShadowPipeline.GetRenderPass("Shadow").GetDepthAttachment();
	//auto& selectedOutput = shading;

	//cmd.TransitionImageLayout(selectedOutput.Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	cmd.TransitionImageLayout(vk.DrawImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	//cmd.CopyImageToImage(selectedOutput.Image, vk.GetDrawImage());
	cmd.TransitionImageLayout(vk.DrawImage, VK_IMAGE_LAYOUT_GENERAL);
	//cmd.TransitionImageLayout(selectedOutput.Image, VK_IMAGE_LAYOUT_GENERAL);

	//cmd.TransitionImageLayout(shadow.Image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
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

	// TODO: remove this.
	Shaders["basic_vert"] = shaderCompiler.CompileShader("../Resources/Shaders/Vulkan/triangle.vert");
	Shaders["shading_frag"] = shaderCompiler.CompileShader("../Resources/Shaders/Vulkan/shading.frag");
	Shaders["shading_vert"] = shaderCompiler.CompileShader("../Resources/Shaders/Vulkan/shading.vert");
	Shaders["shadow_frag"] = shaderCompiler.CompileShader("../Resources/Shaders/Vulkan/shadow.frag");
	Shaders["shadow_vert"] = shaderCompiler.CompileShader("../Resources/Shaders/Vulkan/shadow.vert");
}

void VkSceneRenderer::CreateDescriptors()
{
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

	std::array<Matrix4, MAX_MODEL_MATRIX> allTransforms;
	std::array<MaterialBufferStruct, MAX_MATERIAL> allMaterials;

	uint32_t currentIndex = 0;
	uint32_t currentMaterialIndex = 0;
	auto view = scene->m_Registry.view<TransformComponent, ModelComponent, VisibilityComponent>();
	for (auto e : view)
	{
		// Check if we've reached the maximum capacity of the array
		if (currentIndex >= MAX_MODEL_MATRIX)
		{
			assert(false);
			break;
		}

		auto [transform, mesh, visibility] = view.get<TransformComponent, ModelComponent, VisibilityComponent>(e);

		if (!mesh.ModelResource || !visibility.Visible)
		{
			continue;
		}

		allTransforms[currentIndex] = transform.GetGlobalTransform();

		Entity entity = Entity((entt::entity)e, scene.get());

		GPUResources::Get().ModelMatrixMapping[entity.GetID()] = currentIndex;

		for (auto& m : mesh.ModelResource->GetMeshes())
		{
			Ref<Material> material = m->GetMaterial();
			if (!material)
			{
				continue;
			}

			auto& res = GPUResources::Get();
			// TODO: Avoid duplicated materials
			MaterialBufferStruct materialBuffer = {};
			materialBuffer.hasAlbedo = material->HasAlbedo();
			materialBuffer.albedo = material->data.m_AlbedoColor;
			materialBuffer.hasNormal = material->HasNormal();
			materialBuffer.hasMetalness = material->HasMetalness();
			materialBuffer.metalnessValue = material->data.u_MetalnessValue;
			materialBuffer.hasAO = material->HasAO();
			materialBuffer.aoValue = material->data.u_AOValue;
			materialBuffer.hasRoughness = material->HasRoughness();
			materialBuffer.roughnessValue = material->data.u_RoughnessValue;
			materialBuffer.albedoTextureId = material->HasAlbedo() ? res.GetBindlessTextureID(material->AlbedoImage) : 0;
			materialBuffer.normalTextureId = material->HasNormal() ? res.GetBindlessTextureID(material->NormalImage) : 0;
			materialBuffer.metalnessTextureId = material->HasMetalness() ? res.GetBindlessTextureID(material->MetalnessImage) : 0;
			materialBuffer.aoTextureId = material->HasAO() ? res.GetBindlessTextureID(material->AOImage) : 0;
			materialBuffer.roughnessTextureId = material->HasRoughness() ? res.GetBindlessTextureID(material->RoughnessImage) : 0;
			allMaterials[currentMaterialIndex] = materialBuffer;
			GPUResources::Get().MeshMaterialMapping[m->GetVkMesh()->GetID()] = currentMaterialIndex;

			currentMaterialIndex++;
		}

		currentIndex++;
	}

	currentIndex = 0;
	LightData directionalLight = {};
	directionalLight.castShadow = false;
	directionalLight.color = Vector4(2.0f, 2.0f, 2.0f, 1.0f);
	directionalLight.position = Vector3(0.0f, 0.0f, 0.0f);
	directionalLight.type = LightType::Directional;
	std::array<LightData, MAX_LIGHTS> allLights;
	auto lightView = scene->m_Registry.view<TransformComponent, LightComponent>();
	for (auto e : lightView)
	{
		// Check if we've reached the maximum capacity of the array
		if (currentIndex >= MAX_LIGHTS)
		{
			assert(false);
			break;
		}

		auto [transform, lightComp] = lightView.get<TransformComponent, LightComponent>(e);

		Vector3 direction = transform.GetGlobalRotation() * Vector3(0, 0, -1);
		lightComp.Direction = direction;

		LightData light = {};
		light.position = Vector3(transform.GetGlobalTransform()[3]);
		light.direction = direction;
		light.outerConeAngle = glm::cos(Rad(lightComp.OuterCutoff));
		light.innerConeAngle = glm::cos(Rad(lightComp.Cutoff));
		light.type = lightComp.Type;
		light.color = Vector4(lightComp.Color * lightComp.Strength, 1.0);
		light.castShadow = lightComp.CastShadows;
		for (int i = 0; i < CSM_AMOUNT; i++)
		{
			light.transformId[i] = GPUResources::Get().GetBindlessCameraID(lightComp.m_LightViews[i].CameraID);
			light.shadowMapTextureId[i] = GPUResources::Get().GetBindlessTextureID(lightComp.LightMapID);
		}

		allLights[currentIndex] = light;

		currentIndex++;
	}

	auto& gpu = GPUResources::Get();
	gpu.ModelTransforms = ModelData{ allTransforms };
	gpu.MaterialDataContainer = MaterialData{ allMaterials };
	gpu.LightDataContainerArray = LightDataContainer{ allLights };

	//shadingPushConstant.LightCount = currentIndex;

	// Copy CSM split depths
	for (int i = 0; i < CSM_AMOUNT; i++)
	{
		//shadingPushConstant.CascadeSplits[i] = LightComponent::mCascadeSplitDepth[i];
	}
}

void VkSceneRenderer::UpdateTransformBuffer()
{

}
	