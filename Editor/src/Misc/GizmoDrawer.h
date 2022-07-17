#pragma once
#include <src/Core/Core.h>
#include <src/Rendering/Buffers/VertexArray.h>
#include <src/Rendering/Buffers/VertexBuffer.h>
#include <src/Rendering/Shaders/ShaderManager.h>
#include <src/Rendering/Vertex.h>
#include <src/Scene/Scene.h>
#include <src/Resource/Model.h>

using namespace Nuake;

class GizmoDrawer
{
private:
	std::map<std::string, Ref<Model>> _gizmos;

	const std::vector<LineVertex> vertices
	{
		LineVertex {{10000.f, 0.0f, 0.0f},  {1.f, 0.f, 0.f, 1.f}},
		LineVertex {{0.0f,    0.0f, 0.0f }, {1.f, 0.f, 0.f, 1.f}},
		LineVertex {{0.f,      0.f, 10000.f }, {0.f, 1.f, 0.f, 1.f}},
		LineVertex {{0.0f,    0.0f, 0.0f }, {0.f, 1.f, 0.f, 1.f}},
		LineVertex {{0.f,  10000.f, 0.0f }, {0.f, 0.f, 1.f, 1.f}},
		LineVertex {{0.0f,    0.0f, 0.0f }, {0.f, 0.f, 1.f, 1.f}}
	};

	Ref<VertexArray> mAxisLineBuffer;
	Ref<VertexBuffer> mAxisLineVertexBuffer;
	Shader* mLineShader;

public:
	GizmoDrawer();
	~GizmoDrawer() = default;

	void DrawGizmos(Ref<Scene> scene);
};