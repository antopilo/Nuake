#pragma once
#include "src/Core/Maths.h"

namespace Nuake
{
	class Vertex
	{
	public:
		Vector3 position;
		Vector2 uv;
		Vector3 normal;
		Vector3 tangent;
		Vector3 bitangent;
		float texture;
	};

	struct LineVertex
	{
		Vector3 position;
		Vector4 color;
	};
}

