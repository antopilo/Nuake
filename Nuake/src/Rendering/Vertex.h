#pragma once
#include "src/Core/Maths.h"

namespace Nuake
{
	struct Vertex
	{
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

