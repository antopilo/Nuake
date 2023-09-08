#include "GizmoDrawer.h"
#include <src/Rendering/Buffers/VertexBuffer.h>

#include <src/Scene/Components/LightComponent.h>
#include <src/Scene/Components/CameraComponent.h>
#include <src/Scene/Components/SphereCollider.h>
#include <src/Scene/Components/CharacterControllerComponent.h>
#include <src/Scene/Components/BoxCollider.h>

#include <src/Resource/ModelLoader.h>
#include <src/Rendering/RenderList.h>

#include <dependencies/GLEW/include/GL/glew.h>

#include "src/Scene/Components/CapsuleColliderComponent.h"
#include <src/Scene/Components/CylinderColliderComponent.h>
#include <src/Scene/Components/MeshCollider.h>
#include <src/Scene/Components/ModelComponent.h>
#include <src/Scene/Components/ParticleEmitterComponent.h>


GizmoDrawer::GizmoDrawer()
{
	mLineShader = Nuake::ShaderManager::GetShader("resources/Shaders/line.shader");

	mAxisLineBuffer = CreateRef<Nuake::VertexArray>();
	mAxisLineBuffer->Bind();

	mAxisLineVertexBuffer = CreateRef<VertexBuffer>(vertices.data(), vertices.size() * sizeof(Nuake::LineVertex));
	auto vblayout = CreateRef<VertexBufferLayout>();
	vblayout->Push<float>(3);
	vblayout->Push<float>(4);

	mAxisLineBuffer->AddBuffer(*mAxisLineVertexBuffer, *vblayout);

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

	mBoxBuffer = CreateRef<Nuake::VertexArray>();
	mBoxBuffer->Bind();

	mBoxVertexBuffer = CreateRef<VertexBuffer>(mBoxVertices.data(), mBoxVertices.size() * sizeof(Nuake::LineVertex));

	mBoxBuffer->AddBuffer(*mBoxVertexBuffer, *vblayout);


	// Load gizmos
	ModelLoader loader;
	_gizmos = std::map<std::string, Ref<Model>>();
	_gizmos["cam"] = loader.LoadModel("resources/Models/Camera.gltf");
	_gizmos["light"] = loader.LoadModel("resources/Models/Light.gltf");
	_gizmos["player"] = loader.LoadModel("resources/Models/Camera.gltf");
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

		circleVertices.push_back(LineVertex{ vert1, Color(1.0, 0, 0.0, 0.5) });
		circleVertices.push_back(LineVertex{ vert2, Color(1.0, 0, 0.0, 0.5) });
	}

	mCircleBuffer = CreateRef<Nuake::VertexArray>();
	mCircleBuffer->Bind();

	mCircleVertexBuffer = CreateRef<VertexBuffer>(circleVertices.data(), circleVertices.size() * sizeof(Nuake::LineVertex));
	auto vblayout = CreateRef<VertexBufferLayout>();
	vblayout->Push<float>(3);
	vblayout->Push<float>(4);

	mCircleBuffer->AddBuffer(*mCircleVertexBuffer, *vblayout);
}

void GizmoDrawer::DrawGizmos(Ref<Scene> scene)
{
	using namespace Nuake;
	//RenderCommand::Disable(RendererEnum::DEPTH_TEST);
	//RenderCommand::Enable(RendererEnum::DEPTH_TEST);
	// Draw Axis lignes.
	{
		mLineShader->Bind();
		mLineShader->SetUniformMat4f("u_View", scene->m_EditorCamera->GetTransform());
		mLineShader->SetUniformMat4f("u_Projection", scene->m_EditorCamera->GetPerspective());

		mAxisLineBuffer->Bind();
		Nuake::RenderCommand::DrawLines(0, 6);
	}

	glLineWidth(1.0f);
	auto boxColliderView = scene->m_Registry.view<TransformComponent, BoxColliderComponent>();
	for (auto e : boxColliderView)
	{
		auto [transform, box] = scene->m_Registry.get<TransformComponent, BoxColliderComponent>(e);

		const Quat& globalRotation = glm::normalize(transform.GetGlobalRotation());
		const Matrix4& rotationMatrix = glm::mat4_cast(globalRotation);

		mLineShader->Bind();
		mLineShader->SetUniformMat4f("u_View", glm::scale(glm::translate(scene->m_EditorCamera->GetTransform(), transform.Translation) * rotationMatrix, box.Size));
		mLineShader->SetUniformMat4f("u_Projection", scene->m_EditorCamera->GetPerspective());

		mBoxBuffer->Bind();
		Nuake::RenderCommand::DrawLines(0, 26);
	}

	auto sphereColliderView = scene->m_Registry.view<TransformComponent, SphereColliderComponent>();
	for (auto e : sphereColliderView)
	{
		auto [transform, sphere] = scene->m_Registry.get<TransformComponent, SphereColliderComponent>(e);
		mLineShader->Bind();
		mLineShader->SetUniformMat4f("u_View", glm::scale(glm::translate(scene->m_EditorCamera->GetTransform(), transform.Translation), Vector3(sphere.Radius)));
		mLineShader->SetUniformMat4f("u_Projection", scene->m_EditorCamera->GetPerspective());

		mCircleBuffer->Bind();
		Nuake::RenderCommand::DrawLines(0, 128);
	}

	auto capsuleColliderView = scene->m_Registry.view<TransformComponent, CapsuleColliderComponent>();
	for (auto e : capsuleColliderView)
	{
		auto& [transform, capsule] = scene->m_Registry.get<TransformComponent, CapsuleColliderComponent>(e);

		const auto entityId = (uint32_t)e;
		if (_CapsuleEntity.find(entityId) == _CapsuleEntity.end())
		{
			_CapsuleEntity[entityId] = CreateScope<CapsuleGizmo>();
		}

		_CapsuleEntity[entityId]->UpdateShape(capsule.Radius, capsule.Height);

		const Quat& globalRotation = glm::normalize(transform.GetGlobalRotation());
		const Matrix4& rotationMatrix = glm::mat4_cast(globalRotation);

		mLineShader->Bind();
		mLineShader->SetUniformMat4f("u_View", glm::translate(scene->m_EditorCamera->GetTransform(), transform.Translation) * rotationMatrix);
		mLineShader->SetUniformMat4f("u_Projection", scene->m_EditorCamera->GetPerspective());

		_CapsuleEntity[entityId]->Bind();
		Nuake::RenderCommand::DrawLines(0, 264);
	}

	auto cylinderColliderView = scene->m_Registry.view<TransformComponent, CylinderColliderComponent>();
	for (auto e : cylinderColliderView)
	{
		auto& [transform, cylinder] = scene->m_Registry.get<TransformComponent, CylinderColliderComponent>(e);

		const auto entityId = (uint32_t)e;
		if (_CylinderEntity.find(entityId) == _CylinderEntity.end())
		{
			_CylinderEntity[entityId] = CreateScope<CylinderGizmo>();
		}

		_CylinderEntity[entityId]->UpdateShape(cylinder.Radius, cylinder.Height);

		const Quat& globalRotation = glm::normalize(transform.GetGlobalRotation());
		const Matrix4& rotationMatrix = glm::mat4_cast(globalRotation);

		mLineShader->Bind();
		mLineShader->SetUniformMat4f("u_View", glm::translate(scene->m_EditorCamera->GetTransform(), transform.Translation) * rotationMatrix);
		mLineShader->SetUniformMat4f("u_Projection", scene->m_EditorCamera->GetPerspective());

		_CylinderEntity[entityId]->Bind();
		Nuake::RenderCommand::DrawLines(0, 264);
	}

	auto particleView = scene->m_Registry.view<TransformComponent, ParticleEmitterComponent>();
	for (auto e : particleView)
	{
		auto [transform, particle] = scene->m_Registry.get<TransformComponent, ParticleEmitterComponent>(e);
		mLineShader->Bind();
		mLineShader->SetUniformMat4f("u_View", glm::scale(glm::translate(scene->m_EditorCamera->GetTransform(), transform.Translation), Vector3(particle.Radius)));
		mLineShader->SetUniformMat4f("u_Projection", scene->m_EditorCamera->GetPerspective());

		mCircleBuffer->Bind();
		Nuake::RenderCommand::DrawLines(0, 128);
	}

	auto meshColliderView = scene->m_Registry.view<TransformComponent, MeshColliderComponent, ModelComponent>();
	for (auto e : meshColliderView)
	{
		auto& [transform, mesh, model] = scene->m_Registry.get<TransformComponent, MeshColliderComponent, ModelComponent>(e);

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

		mLineShader->Bind();
		mLineShader->SetUniformMat4f("u_View", glm::translate(scene->m_EditorCamera->GetTransform(), transform.Translation) * rotationMatrix);
		mLineShader->SetUniformMat4f("u_Projection", scene->m_EditorCamera->GetPerspective());

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		meshes[mesh.SubMesh]->Bind();
		meshes[mesh.SubMesh]->Draw(nullptr, false);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	auto flatShader = ShaderManager::GetShader("resources/Shaders/flat.shader");
	flatShader->Bind();
	flatShader->SetUniformMat4f("u_View", scene->m_EditorCamera->GetTransform());
	flatShader->SetUniformMat4f("u_Projection", scene->m_EditorCamera->GetPerspective());
	flatShader->SetUniform4f("u_Color", 0.5f, 0.5f, 0.5f, 1.0f);
	
	RenderCommand::Enable(RendererEnum::DEPTH_TEST);
	RenderCommand::Enable(RendererEnum::FACE_CULL);
	glCullFace(GL_BACK);

	glLineWidth(1.0f);
	RenderList renderList;

	// Camera
	auto camView = scene->m_Registry.view<TransformComponent, CameraComponent>();
	for (auto e : camView)
	{
		auto& [transform, cam] = scene->m_Registry.get<TransformComponent, CameraComponent>(e);

		renderList.AddToRenderList(_gizmos["cam"]->GetMeshes()[0], transform.GetGlobalTransform());

		auto view = transform.GetGlobalTransform();
		Frustum& frustum = cam.CameraInstance->GetFrustum();
		auto& frustumCorners = frustum.GetPoints();

		constexpr int frustumEdges[12][2] = {
			{0, 1}, {1, 3}, {3, 2}, {2, 0}, // Near plane edges
			{4, 5}, {5, 7}, {7, 6}, {6, 4}, // Far plane edges
			{0, 4}, {1, 5}, {2, 6}, {3, 7}  // Connection lines
		};

		glBegin(GL_LINES);

		for (int i = 0; i < 12; ++i) {
			int startIdx = frustumEdges[i][0];
			int endIdx = frustumEdges[i][1];
			const Vector3& startCorner = view * Vector4(frustumCorners[startIdx], 1.0f);
			const Vector3& endCorner = frustumCorners[endIdx];
			glVertex3f(startCorner.x, startCorner.y, startCorner.z);
			glVertex3f(endCorner.x, endCorner.y, endCorner.z);
		}

		glEnd();

	}

	renderList.Flush(flatShader, true);

	// Lights
	auto lightView = scene->m_Registry.view<TransformComponent, LightComponent>();
	for (auto e : lightView)
	{
		auto [transform, light] = scene->m_Registry.get<TransformComponent, LightComponent>(e);

		flatShader->SetUniformVec4("u_Color", Vector4(light.Color, 1.0f));
		renderList.AddToRenderList(_gizmos["light"]->GetMeshes()[0], transform.GetGlobalTransform());
		renderList.Flush(flatShader, true);
	}
	renderList.Flush(flatShader, true);


	// Player
	auto characterControllerView = scene->m_Registry.view<TransformComponent, CharacterControllerComponent>();
	for (auto e : characterControllerView)
	{
		auto [transformComponent, characterControllerComponent] = scene->m_Registry.get<TransformComponent, CharacterControllerComponent>(e);

		flatShader->SetUniformVec4("u_Color", Vector4(0.0f, 1.0f, 0.4f, 1.0f));

		const auto scaledTransform = glm::scale(transformComponent.GetGlobalTransform(), Vector3(0.25f, 0.25f, 0.25f));
		renderList.AddToRenderList(_gizmos["player"]->GetMeshes()[0], scaledTransform);
		renderList.Flush(flatShader, true);
	}

	renderList.Flush(flatShader, true);

	RenderCommand::Disable(RendererEnum::FACE_CULL);
	RenderCommand::Enable(RendererEnum::DEPTH_TEST);
}