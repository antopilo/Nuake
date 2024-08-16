#pragma once
#include "src/Core/Core.h"
#include <src/Rendering/Vertex.h>
#include <src/Rendering/Buffers/VertexArray.h>

namespace Nuake {

	class BoxGizmo
	{
		std::vector<LineVertex> _Vertices;
		Ref<VertexArray> _VertexArray;
		Ref<VertexBuffer> _VertexBuffer;

	public:
		BoxGizmo() = default;
		~BoxGizmo() = default;

		void UpdateShape(float radius, float height);
		void CreateMesh();
		void Bind();
	};
}