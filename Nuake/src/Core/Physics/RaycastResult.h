#pragma once
#include <glm/ext/vector_float3.hpp>
#include "CharacterController.h"


namespace Nuake
{
	namespace Physics
	{
		
	}


	// result object from raycast.
	struct RaycastResult {
		Vector3 WorldPoint;
		Vector3 LocalPoint;
		Vector3 Normal;
	};
}
