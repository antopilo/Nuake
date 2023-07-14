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
	GenerateCapsuleGizmo();

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
		circleVertices.push_back(LineVertex{ vert1, Color(1.0, 0, 0.7, 1.0) });
		circleVertices.push_back(LineVertex{ vert2, Color(1.0, 0, 0.7, 1.0) });
	}

	mCircleBuffer = CreateRef<Nuake::VertexArray>();
	mCircleBuffer->Bind();

	mCircleVertexBuffer = CreateRef<VertexBuffer>(circleVertices.data(), circleVertices.size() * sizeof(Nuake::LineVertex));
	auto vblayout = CreateRef<VertexBufferLayout>();
	vblayout->Push<float>(3);
	vblayout->Push<float>(4);

	mCircleBuffer->AddBuffer(*mCircleVertexBuffer, *vblayout);
}

void GizmoDrawer::GenerateCapsuleGizmo()
{
	float height = 1.5f;
	float radius = 0.5f;

	float halfHeight = height / 2.0f;

	float bottomCircleHeight = -halfHeight + radius;
	float topCircleHeight = halfHeight - radius;

	// Generate circles
	const float subDivision = 32.0f;
	constexpr const float pi = glm::pi<float>() * 4.0;
	float increment = pi / subDivision;
	for (int i = 0; i < subDivision * 2.0; i++)
	{
		float current = increment * (i);
		float x = glm::cos(current) * radius;
		float z = glm::sin(current) * radius;

		current = increment * (i + 1);
		float x2 = glm::cos(current) * radius;
		float z2 = glm::sin(current) * radius;

		Vector3 vert1, vert2;
		if (i < subDivision)
		{
			vert1 = Vector3(x, topCircleHeight, z);
			vert2 = Vector3(x2, topCircleHeight, z2);
		}
		else
		{
			vert1 = Vector3(x, bottomCircleHeight, z);
			vert2 = Vector3(x2, bottomCircleHeight, z2);
		}

		capsuleVertices.push_back(LineVertex{ vert1, Color(1.0, 0, 0.7, 1.0) });
		capsuleVertices.push_back(LineVertex{ vert2, Color(1.0, 0, 0.7, 1.0) });
	}

	for (int i = 0; i < subDivision * 2.0; i++)
	{
		float current = increment * (i);
		float x = glm::cos(current) * radius;
		float z = glm::sin(current) * radius;

		current = increment * (i + 1);
		float x2 = glm::cos(current) * radius;
		float z2 = glm::sin(current) * radius;

		float heightOffset = topCircleHeight;
		if (z < 0.0)
		{
			heightOffset = bottomCircleHeight;
		}

		Vector3 vert1, vert2;
		if (i < subDivision)
		{
			vert1 = Vector3(x, z + heightOffset, 0);
			vert2 = Vector3(x2, z2 + heightOffset, 0);
		}
		else
		{
			vert1 = Vector3(0, z + heightOffset, x);
			vert2 = Vector3(0, z2 + heightOffset, x2 );
		}
		
		capsuleVertices.push_back(LineVertex{ vert1, Color(1.0, 0, 0.7, 1.0) });
		capsuleVertices.push_back(LineVertex{ vert2, Color(1.0, 0, 0.7, 1.0) });
	}

	capsuleVertices.push_back(LineVertex{ Vector3(radius, bottomCircleHeight, 0), Color(1.0, 0, 0.7, 1.0)});
	capsuleVertices.push_back(LineVertex{ Vector3(radius, topCircleHeight, 0), Color(1.0, 0, 0.7, 1.0) });

	capsuleVertices.push_back(LineVertex{ Vector3(-radius, bottomCircleHeight, 0), Color(1.0, 0, 0.7, 1.0) });
	capsuleVertices.push_back(LineVertex{ Vector3(-radius, topCircleHeight, 0), Color(1.0, 0, 0.7, 1.0) });

	capsuleVertices.push_back(LineVertex{ Vector3(0, bottomCircleHeight, radius), Color(1.0, 0, 0.7, 1.0) });
	capsuleVertices.push_back(LineVertex{ Vector3(0, topCircleHeight, radius), Color(1.0, 0, 0.7, 1.0) });

	capsuleVertices.push_back(LineVertex{ Vector3(0, bottomCircleHeight, -radius), Color(1.0, 0, 0.7, 1.0) });
	capsuleVertices.push_back(LineVertex{ Vector3(0, topCircleHeight, -radius), Color(1.0, 0, 0.7, 1.0) });

	mCapsuleBuffer = CreateRef<Nuake::VertexArray>();
	mCapsuleBuffer->Bind();

	mCapsuleVertexBuffer = CreateRef<VertexBuffer>(capsuleVertices.data(), capsuleVertices.size() * sizeof(Nuake::LineVertex));
	auto vblayout = CreateRef<VertexBufferLayout>();
	vblayout->Push<float>(3);
	vblayout->Push<float>(4);

	mCapsuleBuffer->AddBuffer(*mCapsuleVertexBuffer, *vblayout);
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
		auto [transform, capsule] = scene->m_Registry.get<TransformComponent, CapsuleColliderComponent>(e);
		mLineShader->Bind();
		mLineShader->SetUniformMat4f("u_View", glm::translate(scene->m_EditorCamera->GetTransform(), transform.Translation));
		mLineShader->SetUniformMat4f("u_Projection", scene->m_EditorCamera->GetPerspective());

		mCapsuleBuffer->Bind();
		Nuake::RenderCommand::DrawLines(0, 264);
	}

	auto flatShader = ShaderManager::GetShader("resources/Shaders/flat.shader");
	flatShader->Bind();
	flatShader->SetUniformMat4f("u_View", scene->m_EditorCamera->GetTransform());
	flatShader->SetUniformMat4f("u_Projection", scene->m_EditorCamera->GetPerspective());
	flatShader->SetUniform4f("u_Color", 0.5f, 0.5f, 0.5f, 1.0f);
	
	RenderCommand::Enable(RendererEnum::DEPTH_TEST);
	RenderCommand::Enable(RendererEnum::FACE_CULL);
	glCullFace(GL_BACK);

	RenderList renderList;

	// Camera
	auto camView = scene->m_Registry.view<TransformComponent, CameraComponent>();
	for (auto e : camView)
	{
		auto [transform, cam] = scene->m_Registry.get<TransformComponent, CameraComponent>(e);

		renderList.AddToRenderList(_gizmos["cam"]->GetMeshes()[0], transform.GetGlobalTransform());
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