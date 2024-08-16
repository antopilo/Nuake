#pragma once
#include "src/Core/Core.h"
#include "src/Rendering/Vertex.h"
#include "src/Rendering/Buffers/VertexBuffer.h"
#include "src/Rendering/Buffers/VertexArray.h"

#include <vector>

namespace Nuake {

	class CapsuleGizmo
	{
	private:
		std::vector<LineVertex> _CapsuleVertices;
		Ref<VertexArray> _CapsuleBuffer;
		Ref<VertexBuffer> _CapsuleVertexBuffer;

		float _Radius;
		float _Height;

	public:
		CapsuleGizmo();
		~CapsuleGizmo() = default;

		void UpdateShape(float radius, float height);
		void CreateMesh();
		void Bind();
	};
}
