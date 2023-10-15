#pragma once
#include <src/Core/Core.h>
#include <src/Rendering/Buffers/VertexArray.h>
#include <src/Rendering/Buffers/VertexBuffer.h>
#include <src/Rendering/Shaders/ShaderManager.h>
#include <src/Rendering/Vertex.h>
#include <src/Scene/Scene.h>
#include <src/Resource/Model.h>
#include <src/Core/Physics/PhysicsShapes.h>

#include "Gizmos/CapsuleGizmo.h"
#include "Gizmos/CylinderGizmo.h"
#include "../Windows/EditorInterface.h"


using namespace Nuake;

class GizmoDrawer
{
private:
	EditorInterface* m_Editor;
	Shader* m_LineShader;

	std::map<std::string, Ref<Model>> m_Gizmos;
	std::map<uint32_t, Scope<CapsuleGizmo>> m_CapsuleGizmo;
	std::map<uint32_t, Scope<CylinderGizmo>> m_CylinderGizmo;

	const std::vector<LineVertex> m_Vertices
	{
		LineVertex {{10000.f, 0.0f, 0.0f},  {1.f, 0.f, 0.f, 1.f}},
		LineVertex {{0.0f,    0.0f, 0.0f }, {1.f, 0.f, 0.f, 1.f}},
		LineVertex {{0.f,      0.f, 10000.f }, {0.f, 0.f, 1.f, 1.f}},
		LineVertex {{0.0f,    0.0f, 0.0f }, {0.f, 0.f, 1.f, 1.f}},
		LineVertex {{0.f,  10000.f, 0.0f }, {0.f, 1.f, 0.f, 1.f}},
		LineVertex {{0.0f,    0.0f, 0.0f }, {0.f, 1.f, 0.f, 1.f}}
	};

	std::vector<LineVertex> m_CircleVertices;
	Ref<VertexArray> m_CircleBuffer;
	Ref<VertexBuffer> m_CircleVertexBuffer;

	std::vector<LineVertex> m_BoxVertices;
	Ref<VertexArray> m_BoxBuffer;
	Ref<VertexBuffer> m_BoxVertexBuffer;

	Ref<VertexArray> m_AxisLineBuffer;
	Ref<VertexBuffer> m_AxisLineVertexBuffer;


public:
	GizmoDrawer(EditorInterface* editor);

	GizmoDrawer() = default;
	~GizmoDrawer() = default;

	void DrawGizmos(Ref<Scene> scene, bool occluded);
	void DrawAxis(Ref<Scene> scene, bool occlude);

private:
	void GenerateSphereGizmo();
	bool IsEntityInSelection(Nuake::Entity entity);

};