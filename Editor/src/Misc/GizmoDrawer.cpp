#include "GizmoDrawer.h"
#include <src/Rendering/Buffers/VertexBuffer.h>
#include <src/Scene/Components/CameraComponent.h>
#include <src/Resource/ModelLoader.h>
#include <src/Rendering/RenderList.h>
#include <dependencies/GLEW/include/GL/glew.h>
#include <src/Scene/Components/LightComponent.h>

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

	// Load gizmos
	ModelLoader loader;
	_gizmos = std::map<std::string, Ref<Model>>();
	_gizmos["cam"] = loader.LoadModel("resources/Models/Camera.gltf");
	_gizmos["light"] = loader.LoadModel("resources/Models/Light.gltf");
}

void GizmoDrawer::DrawGizmos(Ref<Scene> scene)
{
	using namespace Nuake;
	RenderCommand::Disable(RendererEnum::DEPTH_TEST);

	// Draw Axis lignes.
	{
		mLineShader->Bind();
		mLineShader->SetUniformMat4f("u_View", scene->m_EditorCamera->GetTransform());
		mLineShader->SetUniformMat4f("u_Projection", scene->m_EditorCamera->GetPerspective());

		mAxisLineBuffer->Bind();
		Nuake::RenderCommand::DrawLines(0, 6);
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
	auto camView = scene->m_Registry.view<TransformComponent, CameraComponent>();
	for (auto e : camView)
	{
		auto [transform, cam] = scene->m_Registry.get<TransformComponent, CameraComponent>(e);

		renderList.AddToRenderList(_gizmos["cam"]->GetMeshes()[0], transform.GetGlobalTransform());
	}
	renderList.Flush(flatShader, true);

	auto lightView = scene->m_Registry.view<TransformComponent, LightComponent>();
	for (auto e : lightView)
	{
		auto [transform, light] = scene->m_Registry.get<TransformComponent, LightComponent>(e);

		flatShader->SetUniformVec4("u_Color", Vector4(light.Color, 1.0f));
		renderList.AddToRenderList(_gizmos["light"]->GetMeshes()[0], transform.GetGlobalTransform());
		renderList.Flush(flatShader, true);
	}

	renderList.Flush(flatShader, true);

	RenderCommand::Enable(RendererEnum::DEPTH_TEST);
}