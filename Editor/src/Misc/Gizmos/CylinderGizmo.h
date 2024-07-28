#pragma once
#include <src/Core/Core.h>
#include "src/Core/Maths.h"
#include <src/Rendering/Vertex.h>
#include <src/Rendering/Buffers/VertexBuffer.h>
#include <src/Rendering/Buffers/VertexArray.h>

#include <vector>

class CylinderGizmo
{
private:
	std::vector<Nuake::LineVertex> _CylinderVertices;
	Ref<Nuake::VertexArray> _CylinderBuffer;
	Ref<Nuake::VertexBuffer> _CylinderVertexBuffer;

	float _Radius;
	float _Height;
	Nuake::Color _Color;

public:
	CylinderGizmo(Nuake::Color color = Nuake::Color(1, 0, 0, 1));
	~CylinderGizmo() = default;

	void UpdateShape(float radius, float height, Nuake::Color color = Nuake::Color(1, 0, 0, 1));
	void CreateMesh();
	void Bind();
};