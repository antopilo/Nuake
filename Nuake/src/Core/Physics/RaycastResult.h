#pragma once
#include <glm/ext/vector_float3.hpp>

// result object from raycast.
struct RaycastResult {
	glm::vec3 WorldPoint;
	glm::vec3 LocalPoint;
	glm::vec3 Normal;
};