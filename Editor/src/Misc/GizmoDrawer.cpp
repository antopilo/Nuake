#include "GizmoDrawer.h"

#include <src/Rendering/SceneRenderer.h>
#include <src/Rendering/Buffers/VertexBuffer.h>

#include <src/Scene/Components/LightComponent.h>
#include <src/Scene/Components/CameraComponent.h>
#include <src/Scene/Components/SphereCollider.h>
#include <src/Scene/Components/CharacterControllerComponent.h>
#include <src/Scene/Components/BoxCollider.h>

#include <src/Resource/ModelLoader.h>
#include <src/Rendering/RenderList.h>
#include <src/Rendering/Renderer.h>

#include <glad/glad.h>

#include "src/Scene/Components/CapsuleColliderComponent.h"
#include <src/Scene/Components/CylinderColliderComponent.h>
#include <src/Scene/Components/MeshCollider.h>
#include <src/Scene/Components/ModelComponent.h>
#include <src/Scene/Components/ParticleEmitterComponent.h>
#include <src/Scene/Components/BoneComponent.h>
#include <src/Scene/Components/AudioEmitterComponent.h>


GizmoDrawer::GizmoDrawer(EditorInterface* editor)
{
	m_LineShader = Nuake::ShaderManager::GetShader("Resources/Shaders/line.shader");

	m_AxisLineBuffer = CreateRef<Nuake::VertexArray>();
	m_AxisLineBuffer->Bind();

	m_AxisLineVertexBuffer = CreateRef<VertexBuffer>(m_Vertices.data(), m_Vertices.size() * sizeof(Nuake::LineVertex));
	auto vblayout = CreateRef<VertexBufferLayout>();
	vblayout->Push<float>(3);
	vblayout->Push<float>(4);

	m_AxisLineBuffer->AddBuffer(*m_AxisLineVertexBuffer, *vblayout);

	GenerateSphereGizmo();

	// Box
	const Color cubeColor = Color(1, 0, 0, 0.5f);
	std::vector<LineVertex> mBoxVertices = 
	{
		LineVertex{Vector3(-1.f, -1.f, -1.f), cubeColor},
		LineVertex{Vector3(1.0f, -1.f, -1.f), cubeColor},
		LineVertex{Vector3(-1.f, -1.f, 1.f), cubeColor},
		LineVertex{Vector3(1.0f, -1.f, 1.f), cubeColor},
		LineVertex{Vector3(-1.f, 1.f, -1.f), cubeColor},
		LineVertex{Vector3(1.0f, 1.f, -1.f), cubeColor},
		LineVertex{Vector3(-1.f, 1.f, 1.f), cubeColor},
		LineVertex{Vector3(1.0f, 1.f, 1.f), cubeColor},

		LineVertex{Vector3(-1.f, -1.f, -1.f), cubeColor},
		LineVertex{Vector3(-1.0f, -1.f, 1.f), cubeColor},
		LineVertex{Vector3(1.f, -1.f, -1.f), cubeColor},
		LineVertex{Vector3(1.0f, -1.f, 1.f), cubeColor},
		LineVertex{Vector3(-1.f, 1.f, -1.f), cubeColor},
		LineVertex{Vector3(-1.0f, 1.f, 1.f), cubeColor},
		LineVertex{Vector3(1.f, 1.f, -1.f), cubeColor},
		LineVertex{Vector3(1.0f, 1.f, 1.f), cubeColor},

		LineVertex{Vector3(-1.0f, -1.0f, -1.f), cubeColor},
		LineVertex{Vector3(-1.f, 1.0f, -1.f), cubeColor},
		LineVertex{Vector3(1.0f, -1.0f, -1.f), cubeColor},
		LineVertex{Vector3(1.f, 1.0f, -1.f), cubeColor},
		LineVertex{Vector3(-1.0f, -1.0f, 1.f), cubeColor},
		LineVertex{Vector3(-1.f, 1.0f, 1.f), cubeColor},
		LineVertex{Vector3(1.0f, -1.0f, 1.f), cubeColor},
		LineVertex{Vector3(1.0f, 1.0f, 1.f), cubeColor}
	};

	m_BoxBuffer = CreateRef<Nuake::VertexArray>();
	m_BoxBuffer->Bind();

	m_BoxVertexBuffer = CreateRef<VertexBuffer>(mBoxVertices.data(), mBoxVertices.size() * sizeof(Nuake::LineVertex));

	m_BoxBuffer->AddBuffer(*m_BoxVertexBuffer, *vblayout);

	// Load gizmos
	ModelLoader loader;
	m_Gizmos = std::map<std::string, Ref<Model>>();
	//_gizmos["cam"] = loader.LoadModel("Resources/Models/Camera.gltf");
	//_gizmos["light"] = loader.LoadModel("Resources/Models/Light.gltf");
	//_gizmos["player"] = loader.LoadModel("Resources/Models/Camera.gltf");

	m_Editor = editor;
}

void GizmoDrawer::GenerateSphereGizmo()
{
	const float subDivision = 32.0f;

	
	constexpr const float pi = glm::pi<float>() * 4.0;
	float increment = pi / subDivision;
	for (int i = 0; i < subDivision * 2.0; i++)
	{
		float current = increment * (i);
		float x = glm::cos(current) * 1.0;
		float z = glm::sin(current) * 1.0;

		current = increment * (i + 1);
		float x2 = glm::cos(current) * 1.0;
		float z2 = glm::sin(current) * 1.0;

		Vector3 vert1, vert2;
		if (i < subDivision)
		{
			vert1 = Vector3(x, z, 0);
			vert2 = Vector3(x2, z2, 0);
		}
		else
		{
			vert1 = Vector3(x, 0, z);
			vert2 = Vector3(x2, 0, z2);
		}

		m_CircleVertices.push_back(LineVertex{ vert1, Color(1.0, 0, 0.0, 0.5) });
		m_CircleVertices.push_back(LineVertex{ vert2, Color(1.0, 0, 0.0, 0.5) });
	}

	m_CircleBuffer = CreateRef<Nuake::VertexArray>();
	m_CircleBuffer->Bind();

	m_CircleVertexBuffer = CreateRef<VertexBuffer>(m_CircleVertices.data(), m_CircleVertices.size() * sizeof(Nuake::LineVertex));
	auto vblayout = CreateRef<VertexBufferLayout>();
	vblayout->Push<float>(3);
	vblayout->Push<float>(4);

	m_CircleBuffer->AddBuffer(*m_CircleVertexBuffer, *vblayout);
}

bool GizmoDrawer::IsEntityInSelection(Nuake::Entity entity)
{
	if (m_Editor->Selection.Type != EditorSelectionType::Entity)
	{
		return false;
	}

	using namespace Nuake;
	const Nuake::Entity selectedEntity = m_Editor->Selection.Entity;

	if (selectedEntity.GetID() == entity.GetID())
	{
		return true;
	}

	auto& parentComponent = entity.GetComponent<ParentComponent>();
	if (!parentComponent.HasParent)
	{
		return false;
	}

	if (IsEntityInSelection(parentComponent.Parent))
	{
		return true;
	}

	return false;
}

void GizmoDrawer::DrawGizmos(Ref<Scene> scene, bool occluded)
{
	using namespace Nuake;
	//RenderCommand::Disable(RendererEnum::DEPTH_TEST);
	RenderCommand::Enable(RendererEnum::DEPTH_TEST);
	// Draw Axis lignes.
	//glDepthFunc(GL_ALWAYS); // Disable built-in depth testing
	//glDepthMask(false);     // Disable writing to the depth buffer
	{
		m_LineShader->Bind();
		m_LineShader->SetUniformMat4f("u_View", scene->m_EditorCamera->GetTransform());
		m_LineShader->SetUniformMat4f("u_Projection", scene->m_EditorCamera->GetPerspective());
		m_LineShader->SetUniform1f("u_Opacity", occluded ? 0.1f : 0.5f);
		m_AxisLineBuffer->Bind();
		Nuake::RenderCommand::DrawLines(0, 6);
	}

	glLineWidth(2.0f);
	auto boxColliderView = scene->m_Registry.view<TransformComponent, BoxColliderComponent>();
	for (auto e : boxColliderView)
	{
		if (!IsEntityInSelection(Nuake::Entity{ (entt::entity)e, scene.get() }))
		{
			continue;
		}

		auto [transform, box] = scene->m_Registry.get<TransformComponent, BoxColliderComponent>(e);

		const Quat& globalRotation = glm::normalize(transform.GetGlobalRotation());
		const Matrix4& rotationMatrix = glm::mat4_cast(globalRotation);

		m_LineShader->Bind();
		m_LineShader->SetUniformMat4f("u_View", glm::scale(glm::translate(scene->m_EditorCamera->GetTransform(), Vector3(transform.GetGlobalTransform()[3])) * rotationMatrix, box.Size));
		m_LineShader->SetUniformMat4f("u_Projection", scene->m_EditorCamera->GetPerspective());

		m_BoxBuffer->Bind();
		Nuake::RenderCommand::DrawLines(0, 26);
	}

	auto sphereColliderView = scene->m_Registry.view<TransformComponent, SphereColliderComponent>();
	for (auto e : sphereColliderView)
	{
		if (!IsEntityInSelection(Nuake::Entity{ (entt::entity)e, scene.get() }))
		{
			continue;
		}

		auto [transform, sphere] = scene->m_Registry.get<TransformComponent, SphereColliderComponent>(e);
		m_LineShader->Bind();
		m_LineShader->SetUniformMat4f("u_View", glm::scale(glm::translate(scene->m_EditorCamera->GetTransform(), Vector3(transform.GetGlobalTransform()[3])), Vector3(sphere.Radius)));
		m_LineShader->SetUniformMat4f("u_Projection", scene->m_EditorCamera->GetPerspective());

		m_CircleBuffer->Bind();
		Nuake::RenderCommand::DrawLines(0, 128);
	}

	auto audioEmitterView = scene->m_Registry.view<TransformComponent, AudioEmitterComponent>();
	for (auto e : audioEmitterView)
	{
		if (!IsEntityInSelection(Nuake::Entity{ (entt::entity)e, scene.get() }))
		{
			continue;
		}

		auto [transform, emitter] = scene->m_Registry.get<TransformComponent, AudioEmitterComponent>(e);

		// We dont need to draw the radius if its not spatialized
		if (!emitter.Spatialized)
		{
			continue;
		}

		Vector3 globalPosition = Vector3(transform.GetGlobalTransform()[3]);
		m_LineShader->Bind();
		m_LineShader->SetUniformMat4f("u_View", glm::scale(glm::translate(scene->m_EditorCamera->GetTransform(), globalPosition), Vector3(emitter.MaxDistance)));
		m_LineShader->SetUniformMat4f("u_Projection", scene->m_EditorCamera->GetPerspective());

		m_CircleBuffer->Bind();
		Nuake::RenderCommand::DrawLines(0, 128);

		m_LineShader->SetUniformMat4f("u_View", glm::scale(glm::translate(scene->m_EditorCamera->GetTransform(), globalPosition), Vector3(emitter.MinDistance)));
		Nuake::RenderCommand::DrawLines(0, 128);
	}

	auto capsuleColliderView = scene->m_Registry.view<TransformComponent, CapsuleColliderComponent>();
	for (auto e : capsuleColliderView)
	{
		if (!IsEntityInSelection(Nuake::Entity{ (entt::entity)e, scene.get() }))
		{
			continue;
		}

		auto [transform, capsule] = scene->m_Registry.get<TransformComponent, CapsuleColliderComponent>(e);

		const auto entityId = (uint32_t)e;
		if (m_CapsuleGizmo.find(entityId) == m_CapsuleGizmo.end())
		{
			m_CapsuleGizmo[entityId] = CreateScope<CapsuleGizmo>();
		}

		m_CapsuleGizmo[entityId]->UpdateShape(capsule.Radius, capsule.Height);

		const Quat& globalRotation = glm::normalize(transform.GetGlobalRotation());
		const Matrix4& rotationMatrix = glm::mat4_cast(globalRotation);

		m_LineShader->Bind();
		m_LineShader->SetUniformMat4f("u_View", glm::translate(scene->m_EditorCamera->GetTransform(), Vector3(transform.GetGlobalTransform()[3])) * rotationMatrix);
		m_LineShader->SetUniformMat4f("u_Projection", scene->m_EditorCamera->GetPerspective());

		m_CapsuleGizmo[entityId]->Bind();
		Nuake::RenderCommand::DrawLines(0, 264);
	}

	auto cylinderColliderView = scene->m_Registry.view<TransformComponent, CylinderColliderComponent>();
	for (auto e : cylinderColliderView)
	{
		if (!IsEntityInSelection(Nuake::Entity{ (entt::entity)e, scene.get() }))
		{
			continue;
		}

		auto [transform, cylinder] = scene->m_Registry.get<TransformComponent, CylinderColliderComponent>(e);

		const auto entityId = (uint32_t)e;
		if (m_CylinderGizmo.find(entityId) == m_CylinderGizmo.end())
		{
			m_CylinderGizmo[entityId] = CreateScope<CylinderGizmo>();
		}

		m_CylinderGizmo[entityId]->UpdateShape(cylinder.Radius, cylinder.Height);

		const Quat& globalRotation = glm::normalize(transform.GetGlobalRotation());
		const Matrix4& rotationMatrix = glm::mat4_cast(globalRotation);

		m_LineShader->Bind();
		m_LineShader->SetUniformMat4f("u_View", glm::translate(scene->m_EditorCamera->GetTransform(), Vector3(transform.GetGlobalTransform()[3])) * rotationMatrix);
		m_LineShader->SetUniformMat4f("u_Projection", scene->m_EditorCamera->GetPerspective());

		m_CylinderGizmo[entityId]->Bind();
		Nuake::RenderCommand::DrawLines(0, 264);
	}

	auto particleView = scene->m_Registry.view<TransformComponent, ParticleEmitterComponent>();
	for (auto e : particleView)
	{
		if (!IsEntityInSelection(Nuake::Entity{ (entt::entity)e, scene.get() }))
		{
			continue;
		}

		auto [transform, particle] = scene->m_Registry.get<TransformComponent, ParticleEmitterComponent>(e);
		m_LineShader->Bind();
		m_LineShader->SetUniformMat4f("u_View", glm::scale(glm::translate(scene->m_EditorCamera->GetTransform(), Vector3(transform.GetGlobalTransform()[3])), Vector3(particle.Radius)));
		m_LineShader->SetUniformMat4f("u_Projection", scene->m_EditorCamera->GetPerspective());

		m_CircleBuffer->Bind();
		Nuake::RenderCommand::DrawLines(0, 128);
	}

	auto meshColliderView = scene->m_Registry.view<TransformComponent, MeshColliderComponent, ModelComponent>();
	for (auto e : meshColliderView)
	{
		if (!IsEntityInSelection(Nuake::Entity{ (entt::entity)e, scene.get() }))
		{
			continue;
		}

		auto [transform, mesh, model] = scene->m_Registry.get<TransformComponent, MeshColliderComponent, ModelComponent>(e);

		// Component has no mesh set.
		if (!model.ModelResource)
		{
			continue;
		}

		auto& resource = model.ModelResource;

		const auto& meshes = resource->GetMeshes();
		if (mesh.SubMesh >= meshes.size())
		{
			continue;
		}

		const Quat& globalRotation = glm::normalize(transform.GetGlobalRotation());
		const Matrix4& rotationMatrix = glm::mat4_cast(globalRotation);

		m_LineShader->Bind();
		m_LineShader->SetUniformMat4f("u_View", glm::translate(scene->m_EditorCamera->GetTransform(), Vector3(transform.GetGlobalTransform()[3])) * rotationMatrix);
		m_LineShader->SetUniformMat4f("u_Projection", scene->m_EditorCamera->GetPerspective());

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		meshes[mesh.SubMesh]->Bind();
		meshes[mesh.SubMesh]->Draw(nullptr, false);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	auto flatShader = ShaderManager::GetShader("Resources/Shaders/flat.shader");
	flatShader->Bind();
	flatShader->SetUniformMat4f("u_View", scene->m_EditorCamera->GetTransform());
	flatShader->SetUniformMat4f("u_Projection", scene->m_EditorCamera->GetPerspective());
	flatShader->SetUniform4f("u_Color", 0.5f, 0.5f, 0.5f, 1.0f);
	
	RenderCommand::Enable(RendererEnum::DEPTH_TEST);
	RenderCommand::Enable(RendererEnum::FACE_CULL);
	glCullFace(GL_BACK);

	glLineWidth(1.0f);
	RenderList renderList;

	auto gizmoShader = ShaderManager::GetShader("Resources/Shaders/gizmo.shader");
	gizmoShader->Bind();
	gizmoShader->SetUniformMat4f("u_View", scene->m_EditorCamera->GetTransform());
	gizmoShader->SetUniformMat4f("u_Projection", scene->m_EditorCamera->GetPerspective());
	gizmoShader->SetUniform1f("u_Opacity", occluded ? 0.1f : 0.5f);
	RenderCommand::Disable(RendererEnum::FACE_CULL);

	// Camera
	auto camView = scene->m_Registry.view<TransformComponent, CameraComponent>();
	for (auto e : camView)
	{
		gizmoShader->SetUniformTex("gizmo_texture", TextureManager::Get()->GetTexture("Resources/Gizmos/camera.png").get());
		gizmoShader->SetUniform1i("u_EntityID", (uint32_t)e + 1);
		auto [transform, camera] = scene->m_Registry.get<TransformComponent, CameraComponent>(e);

		auto initialTransform = transform.GetGlobalTransform();
		Matrix4 particleTransform = initialTransform;
		particleTransform = glm::inverse(scene->m_EditorCamera->GetTransform());

		// Translation
		const Vector3& particleGlobalPosition = transform.GetGlobalPosition();
		particleTransform[3] = initialTransform[3];

		particleTransform = glm::scale(particleTransform, Vector3(0.5, 0.5, 0.5));

		renderList.AddToRenderList(Renderer::QuadMesh, particleTransform);
		renderList.Flush(gizmoShader, true);
	}
	

	// Lights
	auto lightView = scene->m_Registry.view<TransformComponent, LightComponent>();
	for (auto e : lightView)
	{
		gizmoShader->SetUniformTex("gizmo_texture", TextureManager::Get()->GetTexture("Resources/Gizmos/light.png").get());
		gizmoShader->SetUniform1i("u_EntityID", (uint32_t)e + 1);
		auto [transform, light] = scene->m_Registry.get<TransformComponent, LightComponent>(e);

		auto initialTransform = transform.GetGlobalTransform();
		Matrix4 particleTransform = initialTransform;
		particleTransform = glm::inverse(scene->m_EditorCamera->GetTransform());

		// Translation
		const Vector3& particleGlobalPosition = transform.GetGlobalPosition();
		particleTransform[3] = initialTransform[3];

		particleTransform = glm::scale(particleTransform, Vector3(0.5, 0.5, 0.5));

		renderList.AddToRenderList(Renderer::QuadMesh, particleTransform);
		renderList.Flush(gizmoShader, true);
	}


	// Player
	auto characterControllerView = scene->m_Registry.view<TransformComponent, CharacterControllerComponent>();
	for (auto e : characterControllerView)
	{
		gizmoShader->SetUniformTex("gizmo_texture", TextureManager::Get()->GetTexture("Resources/Gizmos/player.png").get());
		gizmoShader->SetUniform1i("u_EntityID", static_cast<uint32_t>(e) + 1);
		auto [transform, characterControllerComponent] = scene->m_Registry.get<TransformComponent, CharacterControllerComponent>(e);

		auto initialTransform = transform.GetGlobalTransform();
		Matrix4 particleTransform = initialTransform;
		particleTransform = glm::inverse(scene->m_EditorCamera->GetTransform());

		// Translation
		const Vector3& particleGlobalPosition = transform.GetGlobalPosition();
		particleTransform[3] = initialTransform[3];

		particleTransform = glm::scale(particleTransform, Vector3(0.5, 0.5, 0.5));

		renderList.AddToRenderList(Renderer::QuadMesh, particleTransform);
		renderList.Flush(gizmoShader, true);
	}

	

	// Bones
	auto boneView = scene->m_Registry.view<TransformComponent, BoneComponent>();
	for (auto e : boneView)
	{
		gizmoShader->SetUniformTex("gizmo_texture", TextureManager::Get()->GetTexture("Resources/Gizmos/bone.png").get());
		gizmoShader->SetUniform1i("u_EntityID", static_cast<uint32_t>(e) + 1);
		auto [transform, boneComponent] = scene->m_Registry.get<TransformComponent, BoneComponent>(e);

		auto initialTransform = transform.GetGlobalTransform();
		Matrix4 particleTransform = initialTransform;
		particleTransform = glm::inverse(scene->m_EditorCamera->GetTransform());

		// Translation
		const Vector3& particleGlobalPosition = transform.GetGlobalPosition();
		particleTransform[3] = initialTransform[3];
		particleTransform = glm::scale(particleTransform, Vector3(0.1, 0.1, 0.1));

		renderList.AddToRenderList(Renderer::QuadMesh, particleTransform);
		renderList.Flush(gizmoShader, true);
	}

	

	auto audioView = scene->m_Registry.view<TransformComponent, AudioEmitterComponent>();
	for (auto e : audioView)
	{
		gizmoShader->SetUniformTex("gizmo_texture", TextureManager::Get()->GetTexture("Resources/Gizmos/speaker.png").get());
		gizmoShader->SetUniform1i("u_EntityID", static_cast<uint32_t>(e) + 1);
		auto [transformComponent, audioEmitterComponent] = scene->m_Registry.get<TransformComponent, AudioEmitterComponent>(e);

		auto initialTransform = transformComponent.GetGlobalTransform();
		Matrix4 transform = initialTransform;
		transform = glm::inverse(scene->m_EditorCamera->GetTransform());

		// Translation
		const Vector3& globalPosition = transformComponent.GetGlobalPosition();
		transform[3] = initialTransform[3];
		transform = glm::scale(transform, Vector3(0.5f, 0.5f, 0.5f));

		renderList.AddToRenderList(Renderer::QuadMesh, transform);
		renderList.Flush(gizmoShader, true);
	}

	
	
	// Revert to default depth testing
	//glDepthFunc(GL_LESS);
	//
	//// Revert to default depth buffer writing
	//glDepthMask(true);
	RenderCommand::Enable(RendererEnum::DEPTH_TEST);
}