#pragma once
#include "src/Core/Maths.h"

namespace Nuake
{
	struct VkVertex
	{
		Vector3 position;
		float uv_x;
		Vector3 normal;
		float uv_y;
		Vector4 tangent;
		Vector4 bitangent;
	};

	/*
		Vector3 position;
		Vector2 uv;
		Vector3 normal;
		Vector3 tangent = Vector3(0, 1, 0);
		Vector3 bitangent = Vector3(1, 0, 0);
	*/
}
