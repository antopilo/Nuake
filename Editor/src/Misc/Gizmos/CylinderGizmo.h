#pragma once
#include <src/Core/Core.h>
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

public:
	CylinderGizmo();
	~CylinderGizmo() = default;

	void UpdateShape(float radius, float height);
	void CreateMesh();
	void Bind();
};