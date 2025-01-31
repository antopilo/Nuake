#pragma once
#include <Nuake/Core/Core.h>
#include <Nuake/Rendering/Buffers/VertexArray.h>
#include <Nuake/Rendering/Buffers/VertexBuffer.h>
#include <Nuake/Rendering/Shaders/ShaderManager.h>
#include <Nuake/Rendering/Vertex.h>
#include <Nuake/Scene/Scene.h>
#include <Nuake/Resource/Model.h>
#include <Nuake/Physics/PhysicsShapes.h>

#include "../Windows/EditorInterface.h"
#include "Nuake/AI/NavMeshDebugDrawer.h"

using namespace Nuake;

class GizmoDrawer
{
private:
	EditorInterface* m_Editor;
	Shader* m_LineShader;
	NavMeshDebugDrawer m_DebugDrawer;
	std::map<std::string, Ref<Model>> m_Gizmos;
	std::map<uint32_t, Scope<Nuake::CapsuleGizmo>> m_CapsuleGizmo;
	std::map<uint32_t, Scope<Nuake::CylinderGizmo>> m_CylinderGizmo;

	const Vector3 m_GizmoSize = Vector3(0.25f);

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

	void DrawShapes(Ref<Scene> scene, bool occluded);
	void DrawGizmos(Ref<Scene> scene, bool occluded);
	void DrawAxis(Ref<Scene> scene, bool occluded);
	void DrawNavMesh(Ref<Scene> scene, bool occluded);
private:
	void GenerateSphereGizmo();
	bool IsEntityInSelection(Nuake::Entity entity);

	float GetGizmoScale(const Vector3& camPosition, const Nuake::Vector3& transform);
};