#pragma once
#include "src/Core/Core.h"
#include <src/Rendering/Vertex.h>
#include <src/Rendering/Buffers/VertexArray.h>
#include <src/Rendering/Buffers/VertexBuffer.h>

namespace Nuake {

	class SphereGizmo
	{
	private:
		std::vector<LineVertex> _Vertices;
		Ref<VertexArray> _VertexArray;
		Ref<VertexBuffer> _VertexBuffer;

	public:
		SphereGizmo() = default;
		~SphereGizmo() = default;

		void CreateMesh();
		void Bind();
	};
}