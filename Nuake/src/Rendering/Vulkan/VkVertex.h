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
		Vector4 color;
	};
}
