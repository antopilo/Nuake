#include "VulkanSceneRenderer.h"

#include "Nuake/Rendering/Textures/Material.h"

#include "Nuake/Rendering/Vulkan/Pipeline/RenderPipeline.h"
#include "Nuake/Rendering/Vulkan/SceneRenderPipeline.h"
#include "Nuake/Rendering/Vulkan/ShaderCompiler.h"
#include "Nuake/Rendering/Vulkan/VkMesh.h"
#include "Nuake/Rendering/Vulkan/VkResources.h"
#include "Nuake/Rendering/Vulkan/VkShaderManager.h"
#include "Nuake/Rendering/Vulkan/VulkanAllocator.h"
#include "Nuake/Rendering/Vulkan/VulkanCheck.h"
#include "Nuake/Rendering/Vulkan/VulkanInit.h"
#include "Nuake/Rendering/Vulkan/VulkanRenderer.h"

#include "Nuake/Scene/Scene.h"
#include "Nuake/Scene/Entities/Entity.h"
#include "Nuake/Scene/Components/ModelComponent.h"
#include "Nuake/Scene/Components/CameraComponent.h"

#include <Tracy.hpp>

using namespace Nuake;

Ref<VkMesh> VkSceneRenderer::QuadMesh;
Ref<VkMesh> VkSceneRenderer::BoxMesh;
Ref<VkMesh> VkSceneRenderer::CapsuleMesh;
Ref<VkMesh> VkSceneRenderer::SphereMesh;
Ref<VkMesh> VkSceneRenderer::CylinderMesh;
Ref<VkMesh> VkSceneRenderer::ArrowMesh;
Ref<VkMesh> VkSceneRenderer::ConeMesh;

void VkSceneRenderer::Init()
{
	LoadShaders();

	shadowRenderPipeline = CreateRef<ShadowRenderPipeline>();
	sceneRenderPipeline = CreateRef<SceneRenderPipeline>();

    const std::vector<Vertex> quadVertices 
	{
		{ Vector3(-1.0f,  1.0f, 0.0f), 0.0f, Vector3(0, 0, 1), 1.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
		{ Vector3( 1.0f, 1.0f,  0.0f), 1.0f, Vector3(0, 0, 1), 1.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
		{ Vector3(-1.0f, -1.0f, 0.0f), 0.0f, Vector3(0, 0, 1), 0.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
		{ Vector3( 1.0f, -1.0f, 0.0f), 1.0f, Vector3(0, 0, 1), 0.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
		{ Vector3(-1.0f, -1.0f, 0.0f), 0.0f, Vector3(0, 0, 1), 0.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
		{ Vector3( 1.0f,  1.0f, 0.0f), 1.0f, Vector3(0, 0, 1), 1.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) }
    };

    const std::vector<uint32_t> quadIndices
	{
        5, 4, 3, 2, 1, 0
    };

	QuadMesh = CreateRef<VkMesh>(quadVertices, quadIndices);

	const std::vector<Vertex> boxVertices
	{
		// Front face
		{ Vector3(-1.0f, -1.0f,  1.0f), 0.0f, Vector3(0, 0, 1), 0.0f },
		{ Vector3(1.0f, -1.0f,  1.0f), 1.0f, Vector3(0, 0, 1), 0.0f },
		{ Vector3(1.0f,  1.0f,  1.0f), 1.0f, Vector3(0, 0, 1), 1.0f },
		{ Vector3(-1.0f,  1.0f,  1.0f), 0.0f, Vector3(0, 0, 1), 1.0f },

		// Back face
		{ Vector3(1.0f, -1.0f, -1.0f), 0.0f, Vector3(0, 0, -1), 0.0f },
		{ Vector3(-1.0f, -1.0f, -1.0f), 1.0f, Vector3(0, 0, -1), 0.0f },
		{ Vector3(-1.0f,  1.0f, -1.0f), 1.0f, Vector3(0, 0, -1), 1.0f },
		{ Vector3(1.0f,  1.0f, -1.0f), 0.0f, Vector3(0, 0, -1), 1.0f },

		// Left face
		{ Vector3(-1.0f, -1.0f, -1.0f), 0.0f, Vector3(-1, 0, 0), 0.0f },
		{ Vector3(-1.0f, -1.0f,  1.0f), 1.0f, Vector3(-1, 0, 0), 0.0f },
		{ Vector3(-1.0f,  1.0f,  1.0f), 1.0f, Vector3(-1, 0, 0), 1.0f },
		{ Vector3(-1.0f,  1.0f, -1.0f), 0.0f, Vector3(-1, 0, 0), 1.0f },

		// Right face
		{ Vector3(1.0f, -1.0f,  1.0f), 0.0f, Vector3(1, 0, 0), 0.0f },
		{ Vector3(1.0f, -1.0f, -1.0f), 1.0f, Vector3(1, 0, 0), 0.0f },
		{ Vector3(1.0f,  1.0f, -1.0f), 1.0f, Vector3(1, 0, 0), 1.0f },
		{ Vector3(1.0f,  1.0f,  1.0f), 0.0f, Vector3(1, 0, 0), 1.0f },

		// Top face
		{ Vector3(-1.0f,  1.0f,  1.0f), 0.0f, Vector3(0, 1, 0), 0.0f },
		{ Vector3(1.0f,  1.0f,  1.0f), 1.0f, Vector3(0, 1, 0), 0.0f },
		{ Vector3(1.0f,  1.0f, -1.0f), 1.0f, Vector3(0, 1, 0), 1.0f },
		{ Vector3(-1.0f,  1.0f, -1.0f), 0.0f, Vector3(0, 1, 0), 1.0f },

		// Bottom face
		{ Vector3(-1.0f, -1.0f, -1.0f), 0.0f, Vector3(0, -1, 0), 0.0f },
		{ Vector3(1.0f, -1.0f, -1.0f), 1.0f, Vector3(0, -1, 0), 0.0f },
		{ Vector3(1.0f, -1.0f,  1.0f), 1.0f, Vector3(0, -1, 0), 1.0f },
		{ Vector3(-1.0f, -1.0f,  1.0f), 0.0f, Vector3(0, -1, 0), 1.0f },
	};

	const std::vector<uint32_t> boxIndices
	{
		// Front face
	   0, 1, 1, 2, 2, 3, 3, 0,

	   // Back face
	   4, 5, 5, 6, 6, 7, 7, 4,

	   // Side edges
	   0, 5, 1, 4, 2, 7, 3, 6
	};

	BoxMesh = CreateRef<VkMesh>(boxVertices, boxIndices);

	{
		std::vector<Vertex> arrowVertices;
		std::vector<uint32_t> arrowIndices;

		Vector3 base = Vector3(0.0f, 0.0f, 0.0f);     // Start of arrow
		Vector3 tip = Vector3(0.0f, 1.0f, 0.0f);      // Tip of arrow
		float headSize = 0.2f;
		float shaftLength = 0.8f;
		Vector3 shaftEnd = Vector3(0.0f, shaftLength, 0.0f);

		// Shaft line
		arrowVertices.push_back({ base, 0.0f, Vector3(), 0.0f });       // 0
		arrowVertices.push_back({ shaftEnd, 0.0f, Vector3(), 0.0f });   // 1
		arrowIndices.push_back(0);
		arrowIndices.push_back(1);

		// Arrowhead lines (simple triangle)
		Vector3 left = shaftEnd + Vector3(-headSize, headSize, 0.0f);  // 2
		Vector3 right = shaftEnd + Vector3(headSize, headSize, 0.0f);   // 3
		Vector3 back = shaftEnd + Vector3(0.0f, headSize, headSize);   // 4

		arrowVertices.push_back({ left,  0.0f, Vector3(), 0.0f });
		arrowVertices.push_back({ right, 0.0f, Vector3(), 0.0f });
		arrowVertices.push_back({ back,  0.0f, Vector3(), 0.0f });

		// Arrowhead edges
		arrowIndices.push_back(1); arrowIndices.push_back(2);  // shaftEnd -> left
		arrowIndices.push_back(1); arrowIndices.push_back(3);  // shaftEnd -> right
		arrowIndices.push_back(1); arrowIndices.push_back(4);  // shaftEnd -> back

		ArrowMesh = CreateRef<VkMesh>(arrowVertices, arrowIndices);
	}

	{
		std::vector<Vertex> capsuleVertices;
		std::vector<uint32_t> capsuleIndices;

		const int segments = 16;
		const int hemisphereSegments = 8;
		const float radius = 1.0f;
		const float cylinderHeight = 2.0f;
		const float halfHeight = cylinderHeight * 0.5f;

		// =====
		// Circle rings (top and bottom of cylinder)
		for (int i = 0; i < segments; ++i)
		{
			float angle = (2.0f * glm::pi<float>() * i) / segments;
			float x = cos(angle) * radius;
			float z = sin(angle) * radius;

			// Top ring
			capsuleVertices.push_back({ Vector3(x, +halfHeight, z), 0.0f, Vector3(), 0.0f });

			// Bottom ring
			capsuleVertices.push_back({ Vector3(x, -halfHeight, z), 0.0f, Vector3(), 0.0f });
		}

		// Top/bottom circle outline
		for (int i = 0; i < segments; ++i)
		{
			uint32_t top = i * 2;
			uint32_t topNext = ((i + 1) % segments) * 2;
			uint32_t bot = i * 2 + 1;
			uint32_t botNext = ((i + 1) % segments) * 2 + 1;

			capsuleIndices.push_back(top);
			capsuleIndices.push_back(topNext);

			capsuleIndices.push_back(bot);
			capsuleIndices.push_back(botNext);
		}

		// =====
		// 4 vertical lines at 0°, 90°, 180°, 270°
		for (int i : { 0, segments / 4, segments / 2, (3 * segments) / 4 })
		{
			uint32_t top = i * 2;
			uint32_t bot = i * 2 + 1;

			capsuleIndices.push_back(top);
			capsuleIndices.push_back(bot);
		}

		// =====
		// Hemisphere arcs (vertical arcs at 4 directions)
		for (int i : { 0, segments / 4, segments / 2, (3 * segments) / 4 })
		{
			float angle = (2.0f * glm::pi<float>() * i) / segments;
			float x = cos(angle);
			float z = sin(angle);

			// Top hemisphere arc
			uint32_t last = capsuleVertices.size();
			for (int j = 0; j <= hemisphereSegments; ++j)
			{
				float theta = (0.5f * glm::pi<float>() * j) / hemisphereSegments; // from 0 to pi/2
				float y = sin(theta) * radius + halfHeight;
				float r = cos(theta) * radius;

				capsuleVertices.push_back({ Vector3(x * r, y, z * r), 0.0f, Vector3(), 0.0f });

				if (j > 0)
				{
					capsuleIndices.push_back(last + j - 1);
					capsuleIndices.push_back(last + j);
				}
			}

			// Bottom hemisphere arc
			last = capsuleVertices.size();
			for (int j = 0; j <= hemisphereSegments; ++j)
			{
				float theta = (0.5f * glm::pi<float>() * j) / hemisphereSegments; // from 0 to pi/2
				float y = -sin(theta) * radius - halfHeight;
				float r = cos(theta) * radius;

				capsuleVertices.push_back({ Vector3(x * r, y, z * r), 0.0f, Vector3(), 0.0f });

				if (j > 0)
				{
					capsuleIndices.push_back(last + j - 1);
					capsuleIndices.push_back(last + j);
				}
			}
		}
		CapsuleMesh = CreateRef<VkMesh>(capsuleVertices, capsuleIndices);
	}
	{
		std::vector<Vertex> sphereVertices;
		std::vector<uint32_t> sphereIndices;

		const int ringSegments = 16;
		const float sphereRadius = 0.5f;

		// XY Ring
		for (int i = 0; i < ringSegments; ++i)
		{
			float angle = (2.0f * glm::pi<float>() * i) / ringSegments;
			float x = cos(angle) * sphereRadius;
			float y = sin(angle) * sphereRadius;
			sphereVertices.push_back({ Vector3(x, y, 0.0f), 0.0f, Vector3(), 0.0f });

			uint32_t curr = i;
			uint32_t next = (i + 1) % ringSegments;
			sphereIndices.push_back(curr);
			sphereIndices.push_back(next);
		}

		// XZ Ring
		uint32_t baseXZ = sphereVertices.size();
		for (int i = 0; i < ringSegments; ++i)
		{
			float angle = (2.0f * glm::pi<float>() * i) / ringSegments;
			float x = cos(angle) * sphereRadius;
			float z = sin(angle) * sphereRadius;
			sphereVertices.push_back({ Vector3(x, 0.0f, z), 0.0f, Vector3(), 0.0f });

			uint32_t curr = baseXZ + i;
			uint32_t next = baseXZ + ((i + 1) % ringSegments);
			sphereIndices.push_back(curr);
			sphereIndices.push_back(next);
		}

		// YZ Ring
		uint32_t baseYZ = sphereVertices.size();
		for (int i = 0; i < ringSegments; ++i)
		{
			float angle = (2.0f * glm::pi<float>() * i) / ringSegments;
			float y = cos(angle) * sphereRadius;
			float z = sin(angle) * sphereRadius;
			sphereVertices.push_back({ Vector3(0.0f, y, z), 0.0f, Vector3(), 0.0f });

			uint32_t curr = baseYZ + i;
			uint32_t next = baseYZ + ((i + 1) % ringSegments);
			sphereIndices.push_back(curr);
			sphereIndices.push_back(next);
		}

		SphereMesh = CreateRef<VkMesh>(sphereVertices, sphereIndices);
	}

	{
		std::vector<Vertex> cylinderVertices;
		std::vector<uint32_t> cylinderIndices;

		const int segments = 16;
		const float radius = 0.25f;
		const float halfHeight = 1.0f;

		// Vertex pairs: top and bottom
		for (int i = 0; i < segments; ++i)
		{
			float angle = (2.0f * glm::pi<float>() * i) / segments;
			float x = cos(angle) * radius;
			float z = sin(angle) * radius;

			// Top ring
			cylinderVertices.push_back({ Vector3(x, +halfHeight, z), 0.0f, Vector3(), 0.0f });

			// Bottom ring
			cylinderVertices.push_back({ Vector3(x, -halfHeight, z), 0.0f, Vector3(), 0.0f });
		}

		// Circle edges (top and bottom)
		for (int i = 0; i < segments; ++i)
		{
			uint32_t topIdx = i * 2;
			uint32_t nextTopIdx = ((i + 1) % segments) * 2;
			uint32_t botIdx = i * 2 + 1;
			uint32_t nextBotIdx = ((i + 1) % segments) * 2 + 1;

			// Top circle
			cylinderIndices.push_back(topIdx);
			cylinderIndices.push_back(nextTopIdx);

			// Bottom circle
			cylinderIndices.push_back(botIdx);
			cylinderIndices.push_back(nextBotIdx);
		}

		// 4 vertical edges at 0, 90, 180, 270 degrees
		for (int i : { 0, segments / 4, segments / 2, (3 * segments) / 4 })
		{
			uint32_t topIdx = i * 2;
			uint32_t botIdx = i * 2 + 1;

			cylinderIndices.push_back(topIdx);
			cylinderIndices.push_back(botIdx);
		}

		// Final creation of the cylinder outline mesh
		CylinderMesh = CreateRef<VkMesh>(cylinderVertices, cylinderIndices);
	}

	{
		std::vector<Vertex> coneVertices;
		std::vector<uint32_t> coneIndices;

		const int segments = 16;
		const float radius = 0.5f;
		const float height = 2.0f;
		const float halfHeight = height * 0.5f;

		// Tip vertex at the top center
		Vector3 tipPosition = Vector3(0.0f, +halfHeight, 0.0f);
		coneVertices.push_back({ tipPosition, 0.0f, Vector3(), 0.0f });
		uint32_t tipIndex = 0;

		// Base ring vertices 
		for (int i = 0; i < segments; ++i)
		{
			float angle = (2.0f * glm::pi<float>() * i) / segments;
			float x = cos(angle) * radius;
			float z = sin(angle) * radius;

			Vector3 pos = Vector3(x, -halfHeight, z);
			coneVertices.push_back({ pos, 0.0f, Vector3(), 0.0f });
		}

		// Circle outline on base
		for (int i = 0; i < segments; ++i)
		{
			uint32_t currIdx = tipIndex + 1 + i;
			uint32_t nextIdx = tipIndex + 1 + ((i + 1) % segments);

			coneIndices.push_back(currIdx);
			coneIndices.push_back(nextIdx);
		}

		// Lines from base to tip
		for (int i = 0; i < segments; ++i)
		{
			uint32_t baseIdx = tipIndex + 1 + i;
			coneIndices.push_back(tipIndex);     // tip
			coneIndices.push_back(baseIdx);      // base
		}

		// Final creation of the cone outline mesh
		ConeMesh = CreateRef<VkMesh>(coneVertices, coneIndices);
	}
}

void VkSceneRenderer::LoadShaders()
{
	// TODO: load embedded shaders in the future
	VkShaderManager& shaderMgr = VkShaderManager::Get();
	ShaderCompiler& shaderCompiler = ShaderCompiler::Get(); 
	shaderMgr.AddShader("basic_frag", shaderCompiler.CompileShader("Resources/Shaders/Vulkan/triangle.frag"));
	shaderMgr.AddShader("basic_vert", shaderCompiler.CompileShader("Resources/Shaders/Vulkan/triangle.vert"));
	shaderMgr.AddShader("shading_frag", shaderCompiler.CompileShader("Resources/Shaders/Vulkan/shading.frag"));
	shaderMgr.AddShader("shading_vert", shaderCompiler.CompileShader("Resources/Shaders/Vulkan/shading.vert"));
	shaderMgr.AddShader("shadow_frag", shaderCompiler.CompileShader("Resources/Shaders/Vulkan/shadow.frag"));
	shaderMgr.AddShader("shadow_vert", shaderCompiler.CompileShader("Resources/Shaders/Vulkan/shadow.vert"));
	shaderMgr.AddShader("tonemap_frag", shaderCompiler.CompileShader("Resources/Shaders/Vulkan/tonemap.frag"));
	shaderMgr.AddShader("tonemap_vert", shaderCompiler.CompileShader("Resources/Shaders/Vulkan/tonemap.vert"));
	shaderMgr.AddShader("outline_frag", shaderCompiler.CompileShader("Resources/Shaders/Vulkan/outline.frag"));
	shaderMgr.AddShader("outline_vert", shaderCompiler.CompileShader("Resources/Shaders/Vulkan/outline.vert"));
	shaderMgr.AddShader("gizmo_frag", shaderCompiler.CompileShader("Resources/Shaders/Vulkan/gizmo.frag"));
	shaderMgr.AddShader("gizmo_vert", shaderCompiler.CompileShader("Resources/Shaders/Vulkan/gizmo.vert"));
	shaderMgr.AddShader("copy_frag", shaderCompiler.CompileShader("Resources/Shaders/Vulkan/copy.frag"));
	shaderMgr.AddShader("copy_vert", shaderCompiler.CompileShader("Resources/Shaders/Vulkan/copy.vert"));
	shaderMgr.AddShader("line_frag", shaderCompiler.CompileShader("Resources/Shaders/Vulkan/line.frag"));
	shaderMgr.AddShader("line_vert", shaderCompiler.CompileShader("Resources/Shaders/Vulkan/line.vert"));
	shaderMgr.AddShader("ssao_frag", shaderCompiler.CompileShader("Resources/Shaders/Vulkan/ssao.frag"));
	shaderMgr.AddShader("ssao_vert", shaderCompiler.CompileShader("Resources/Shaders/Vulkan/ssao.vert"));
	shaderMgr.AddShader("blur_frag", shaderCompiler.CompileShader("Resources/Shaders/Vulkan/blur.frag"));
	shaderMgr.AddShader("blur_vert", shaderCompiler.CompileShader("Resources/Shaders/Vulkan/blur.vert"));
	shaderMgr.AddShader("volumetric_frag", shaderCompiler.CompileShader("Resources/Shaders/Vulkan/volumetric.frag"));
	shaderMgr.AddShader("volumetric_vert", shaderCompiler.CompileShader("Resources/Shaders/Vulkan/volumetric.vert"));
	shaderMgr.AddShader("depth_aware_blur_vert", shaderCompiler.CompileShader("Resources/Shaders/Vulkan/depth_aware_blur.vert"));
	shaderMgr.AddShader("depth_aware_blur_frag", shaderCompiler.CompileShader("Resources/Shaders/Vulkan/depth_aware_blur.frag"));
}

void VkSceneRenderer::PrepareScenes(const std::vector<Ref<Scene>>& scenes, RenderContext inContext)
{
	// Prepare all scenes
	auto& gpu = GPUResources::Get();
	gpu.ClearCameras();

	uint32_t lightCount = 0;
	std::array<LightData, MAX_LIGHTS> allLights;

	uint32_t currentIndex = 0;
	uint32_t currentMaterialIndex = 0;
	std::array<Matrix4, MAX_MODEL_MATRIX> allTransforms;
	std::array<MaterialBufferStruct, MAX_MATERIAL> allMaterials;

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
			Vector3 pos = cameraView.View[3];
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
						.SamplerType = static_cast<int>(material->m_SamplingType),
						.ReceiveShadow = material->m_ReceiveShadows,
						.CastShadow = material->m_CastShadows,
						.Unlit = material->data.u_Unlit,
						.AlphaScissor = static_cast<int>(material->m_AlphaScissor)
					};

					// Save bindless mapping index
					allMaterials[currentMaterialIndex] = std::move(materialBuffer);
					gpu.MeshMaterialMapping[m->GetVkMesh()->GetID()] = currentMaterialIndex;
					currentMaterialIndex++;
				}

				currentIndex++;
			}
		}

		// Lights
		{
			int lightIndexStart = lightCount;
			
			int count = 0;;
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

						if (lightComp.m_ShadowMaps[i])
						{
							light.ShadowMapTextureId[i] = gpu.GetBindlessTextureID(lightComp.m_ShadowMaps[i]->GetID());
						}
					}
				}

				allLights[lightCount] = std::move(light);

				lightCount++;
				count++;
			}

			gpu.SceneLightOffets[scene->ID] = { lightIndexStart, count };
		}

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
	}

	gpu.ModelTransforms = TransformData{ allTransforms };
	gpu.MaterialDataContainer = MaterialData{ allMaterials };
	gpu.LightDataContainerArray = LightDataContainer{ allLights };
	gpu.LightCount = lightCount;

	gpu.UpdateBuffers();

	// TODO: Move shadow map creation into its own pass per scene renderer
	for (auto& scene : scenes)
	{
		PassRenderContext passCtx = { };
		passCtx.scene = scene;
		passCtx.commandBuffer = inContext.CommandBuffer;
		passCtx.resolution = inContext.Size;
		passCtx.cameraID = GPUResources::Get().GetBindlessCameraID(scene->GetCurrentCamera()->ID);

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

void VkSceneRenderer::DrawSceneView(RenderContext inContext)
{
	PassRenderContext passCtx = { };
	passCtx.cameraID = GPUResources::Get().GetBindlessCameraID(inContext.CameraID);
	passCtx.resolution = inContext.Size;
	passCtx.commandBuffer = inContext.CommandBuffer;
	passCtx.scene = inContext.CurrentScene;
	passCtx.selectedEntity = static_cast<float>(inContext.SelectedEntityID);
	if (passCtx.resolution != Vector2{ 1, 1 })
	{
		sceneRenderPipeline->Render(passCtx);
	}

	// in case we just resized
	inContext.CommandBuffer.TransitionImageLayout(inContext.ViewportImage, VK_IMAGE_LAYOUT_GENERAL);
	inContext.CommandBuffer.TransitionImageLayout(sceneRenderPipeline->GetOutput(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	inContext.CommandBuffer.TransitionImageLayout(inContext.ViewportImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	inContext.CommandBuffer.CopyImageToImage(sceneRenderPipeline->GetOutput(), inContext.ViewportImage);
	inContext.CommandBuffer.TransitionImageLayout(inContext.ViewportImage, VK_IMAGE_LAYOUT_GENERAL);
	inContext.CommandBuffer.TransitionImageLayout(sceneRenderPipeline->GetOutput(), VK_IMAGE_LAYOUT_GENERAL);
	inContext.CommandBuffer.TransitionImageLayout(inContext.ViewportImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void VkSceneRenderer::RecreatePipelines()
{
	sceneRenderPipeline->RecreatePipeline();
}