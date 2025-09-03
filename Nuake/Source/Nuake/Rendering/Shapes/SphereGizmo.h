#pragma once
#include "Nuake/Core/Core.h"
#include "Nuake/Rendering/Vertex.h"
#include "Nuake/Rendering/Buffers/VertexArray.h"
#include "Nuake/Rendering/Buffers/VertexBuffer.h"

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