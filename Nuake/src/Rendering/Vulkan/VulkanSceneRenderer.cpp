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

	shadowRenderPipeline = CreateRef<ShadowRenderPipeline>();
	sceneRenderPipeline = CreateRef<SceneRenderPipeline>();

    const std::vector<Vertex> quadVertices 
	{
		{ Vector3(-1.0f,  1.0f, 1.0f), 0.0f, Vector3(0, 0, 1), 1.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
		{ Vector3( 1.0f, 1.0f,  1.0f), 1.0f, Vector3(0, 0, 1), 1.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
		{ Vector3(-1.0f, -1.0f, 1.0f), 0.0f, Vector3(0, 0, 1), 0.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
		{ Vector3( 1.0f, -1.0f, 1.0f), 1.0f, Vector3(0, 0, 1), 0.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
		{ Vector3(-1.0f, -1.0f, 1.0f), 0.0f, Vector3(0, 0, 1), 0.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
		{ Vector3( 1.0f,  1.0f, 1.0f), 1.0f, Vector3(0, 0, 1), 1.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) }
    };

    const std::vector<uint32_t> quadIndices
	{
        5, 4, 3, 2, 1, 0
    };

	QuadMesh = CreateRef<VkMesh>(quadVertices, quadIndices);
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
	shaderMgr.AddShader("tonemap_frag", shaderCompiler.CompileShader("../Resources/Shaders/Vulkan/tonemap.frag"));
	shaderMgr.AddShader("tonemap_vert", shaderCompiler.CompileShader("../Resources/Shaders/Vulkan/tonemap.vert"));
}

void VkSceneRenderer::SetGBufferSize(const Vector2& size)
{
	Context.Size = size;
}

void VkSceneRenderer::PrepareScenes(const std::vector<Ref<Scene>>& scenes, RenderContext inContext)
{
	// Prepare all scenes
	auto& gpu = GPUResources::Get();
	gpu.ClearCameras();

	for (auto& scene : scenes)
	{
		// Prepare scene
		// Cameras
		{
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

		// Lights
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

		// Models
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

				// Get() will load the resource if its not loaded already
				Ref<Model> modelResource = mesh.ModelResource.Get<Model>();
				if (!modelResource || !visibility.Visible)
				{
					continue;
				}

				// Upload transforms to GPU resources
				allTransforms[currentIndex] = transform.GetGlobalTransform();
				gpu.ModelMatrixMapping[Entity((entt::entity)e, scene.get()).GetID()] = currentIndex;

				// Upload mesh material to GPU resources
				for (auto& m : modelResource->GetMeshes())
				{
					// TODO: Avoid duplicated materials
					Ref<Material> material = m->GetMaterial();
					if (!material)
					{
						// insert missing material.
						static Ref<Material> missingMaterial = CreateRef<Material>();
						missingMaterial->AlbedoImage = TextureManager::Get()->GetTexture2("missing_texture")->GetID();

						material = missingMaterial;
					}

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

				currentIndex++;
			}

			gpu.ModelTransforms = ModelData{ allTransforms };
			gpu.MaterialDataContainer = MaterialData{ allMaterials };
		}

		// Lights
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

				if (lightComp.CastShadows && lightComp.Type == LightType::Directional)
				{
					for (int i = 0; i < CSM_AMOUNT; i++)
					{
						light.TransformId[i] = gpu.GetBindlessCameraID(lightComp.m_LightViews[i].CameraID);
						light.ShadowMapTextureId[i] = gpu.GetBindlessTextureID(lightComp.m_ShadowMaps[i]->GetID());
					}
				}

				allLights[lightCount] = std::move(light);

				lightCount++;
			}

			gpu.LightDataContainerArray = LightDataContainer{ allLights };
			gpu.LightCount = lightCount;
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

		for (auto& scene : scenes)
		{
			PassRenderContext passCtx = { };
			passCtx.scene = scene;
			passCtx.commandBuffer = inContext.CommandBuffer;
			passCtx.resolution = inContext.Size;
			passCtx.cameraID = GPUResources::Get().GetBindlessCameraID(inContext.CameraID);

			auto view = scene->m_Registry.view<TransformComponent, LightComponent>();
			for (auto e : view)
			{
				auto [transform, light] = view.get<TransformComponent, LightComponent>(e);

				if (light.Type != LightType::Directional || !light.CastShadows)
				{
					continue;
				}

				light.LightMapIDs.clear();
				for (int i = 0; i < CSM_AMOUNT; i++)
				{
					light.LightMapIDs.push_back(light.m_ShadowMaps[i]->GetID());
					passCtx.cameraID = GPUResources::Get().GetBindlessCameraID(light.m_LightViews[i].CameraID);
					passCtx.resolution = Vector2{ 4096, 4096 };
					shadowRenderPipeline->Render(passCtx, light.m_ShadowMaps[i]);
				}
			}
		}
	}

	
}

void VkSceneRenderer::DrawSceneView(RenderContext inContext)
{
	PassRenderContext passCtx = { };
	passCtx.cameraID = GPUResources::Get().GetBindlessCameraID(inContext.CameraID);
	passCtx.resolution = inContext.Size;
	passCtx.commandBuffer = inContext.CommandBuffer;
	passCtx.scene = inContext.CurrentScene;

	sceneRenderPipeline->Render(passCtx);

	inContext.CommandBuffer.TransitionImageLayout(sceneRenderPipeline->GetOutput(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	inContext.CommandBuffer.TransitionImageLayout(inContext.ViewportImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	inContext.CommandBuffer.CopyImageToImage(sceneRenderPipeline->GetOutput(), inContext.ViewportImage);
	inContext.CommandBuffer.TransitionImageLayout(inContext.ViewportImage, VK_IMAGE_LAYOUT_GENERAL);
	inContext.CommandBuffer.TransitionImageLayout(sceneRenderPipeline->GetOutput(), VK_IMAGE_LAYOUT_GENERAL);
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

	// All transforms & materials
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

			// Get() will load the resource if its not loaded already
			Ref<Model> modelResource = mesh.ModelResource.Get<Model>();
			if (!modelResource || !visibility.Visible)
			{
				continue;
			}

			// Upload transforms to GPU resources
			allTransforms[currentIndex] = transform.GetGlobalTransform();
			gpu.ModelMatrixMapping[Entity((entt::entity)e, scene.get()).GetID()] = currentIndex;

			// Upload mesh material to GPU resources
			for (auto& m : modelResource->GetMeshes())
			{
				// TODO: Avoid duplicated materials
				Ref<Material> material = m->GetMaterial();
				if (!material)
				{
					// insert missing material.
					static Ref<Material> missingMaterial = CreateRef<Material>();
					missingMaterial->AlbedoImage = TextureManager::Get()->GetTexture2("missing_texture")->GetID();

					material = missingMaterial;
				}

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

			if (lightComp.CastShadows && lightComp.Type == LightType::Directional)
			{
				for (int i = 0; i < CSM_AMOUNT; i++)
				{
					light.TransformId[i] = gpu.GetBindlessCameraID(lightComp.m_LightViews[i].CameraID);
					light.ShadowMapTextureId[i] = gpu.GetBindlessTextureID(lightComp.m_ShadowMaps[i]->GetID());
				}
			}

			allLights[lightCount] = std::move(light);

			lightCount++;
		}

		gpu.LightDataContainerArray = LightDataContainer{ allLights };
		gpu.LightCount = lightCount;
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

		if (light.Type != LightType::Directional || !light.CastShadows)
		{
			continue;
		}

		light.LightMapIDs.clear();
		for (int i = 0; i < CSM_AMOUNT; i++)
		{
			light.LightMapIDs.push_back(light.m_ShadowMaps[i]->GetID());
			passCtx.cameraID = GPUResources::Get().GetBindlessCameraID(light.m_LightViews[i].CameraID);
			passCtx.resolution = Vector2{ 4096, 4096 };
			shadowRenderPipeline->Render(passCtx, light.m_ShadowMaps[i]);
		}
	}

	// Set back the camera ID to the actual desired camera.
	passCtx.cameraID = GPUResources::Get().GetBindlessCameraID(inContext.CameraID);
	passCtx.resolution = Context.Size;
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
