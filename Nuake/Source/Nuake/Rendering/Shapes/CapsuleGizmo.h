#pragma once
#include "Nuake/Core/Core.h"
#include "Nuake/Rendering/Vertex.h"
#include "Nuake/Rendering/Buffers/VertexBuffer.h"
#include "Nuake/Rendering/Buffers/VertexArray.h"

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
