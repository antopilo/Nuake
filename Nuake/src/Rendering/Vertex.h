#pragma once
#include "src/Core/Maths.h"

namespace Nuake
{
	struct Vertex
	{
		glm::vec3 position;
		glm::vec2 uv;
		glm::vec3 normal;
		glm::vec3 tangent;
		glm::vec3 bitangent;
		float texture;
	};
}

