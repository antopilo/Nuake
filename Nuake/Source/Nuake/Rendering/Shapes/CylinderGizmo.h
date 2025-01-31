#pragma once
#include "Nuake/Core/Core.h"
#include "Nuake/Core/Maths.h"
#include "Nuake/Rendering/Vertex.h"
#include "Nuake/Rendering/Buffers/VertexBuffer.h"
#include "Nuake/Rendering/Buffers/VertexArray.h"

#include <vector>

namespace Nuake {

	class CylinderGizmo
	{
	private:
		std::vector<LineVertex> _CylinderVertices;
		Ref<VertexArray> _CylinderBuffer;
		Ref<VertexBuffer> _CylinderVertexBuffer;

		float _Radius;
		float _Height;
		Color _Color;

	public:
		CylinderGizmo(Color color = Color(1, 0, 0, 1));
		~CylinderGizmo() = default;

		void UpdateShape(float radius, float height, Color color = Color(1, 0, 0, 1));
		void CreateMesh();
		void Bind();
	};
}
