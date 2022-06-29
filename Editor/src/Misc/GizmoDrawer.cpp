#include "GizmoDrawer.h"
#include <src/Rendering/Buffers/VertexBuffer.h>

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
}

void GizmoDrawer::DrawGizmos(Ref<Scene> scene)
{
	RenderCommand::Disable(RendererEnum::DEPTH_TEST);

	// Draw Axis lignes.
	{
		mLineShader->Bind();
		mLineShader->SetUniformMat4f("u_View", scene->m_EditorCamera->GetTransform());
		mLineShader->SetUniformMat4f("u_Projection", scene->m_EditorCamera->GetPerspective());

		mAxisLineBuffer->Bind();
		Nuake::RenderCommand::DrawLines(0, 6);
	}

	// Draw Cameras Frustum.
	{

	}

	RenderCommand::Enable(RendererEnum::DEPTH_TEST);
}