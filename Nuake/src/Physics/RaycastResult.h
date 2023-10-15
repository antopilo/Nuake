#pragma once
#include <glm/ext/vector_float3.hpp>
#include "CharacterController.h"


namespace Nuake
{
	// result object from raycast.
	struct RaycastResult
	{
		Vector3 WorldPosition;
		float Fraction;
	};
}
