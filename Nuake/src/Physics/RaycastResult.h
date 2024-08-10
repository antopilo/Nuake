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

	struct ShapeCastResult
	{
		Vector3 ImpactPosition;
		float Fraction;
		Vector3 ImpactNormal;
		float Layer;
	};
}
