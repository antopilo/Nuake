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
	};

	const uint32_t MAX_BONE_INFLUENCE = 1;
	struct SkinnedVertex
	{
		Vector3 position;
		Vector2 uv;
		Vector3 normal;
		Vector3 tangent;
		Vector3 bitangent;

		int boneIDs[MAX_BONE_INFLUENCE];
		float weights[MAX_BONE_INFLUENCE];
	};

	struct LineVertex
	{
		Vector3 position;
		Vector4 color;
	};
}

