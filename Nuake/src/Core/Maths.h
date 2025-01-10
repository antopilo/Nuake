#pragma once
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_ROW_MAJOR

#include <glm/ext/vector_float4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <src/Vendors/glm/ext/vector_float2.hpp>
#include <src/Vendors/glm/ext/matrix_float4x4.hpp>
#include "src/Vendors/glm/gtx/matrix_decompose.hpp"
#include <glm/gtx/quaternion.hpp>

namespace Nuake
{
	// Type definitions
	using Vector3 = glm::vec3;
	using Vector2 = glm::vec2;
	using Vector4 = glm::vec4;
	using Quat = glm::quat;
	using Color = glm::vec4;
	using Matrix4 = glm::mat4;
	using Matrix3 = glm::mat3;
#define Rad(degrees) glm::radians(degrees)

	Quat LookAt(Vector3 sourcePoint, Vector3 destPoint);
	Quat CreateFromAxisAngle(Vector3 axis, float angle);
	Quat QuatFromEuler(float x, float y, float z);
	Vector3 QuatToDirection(const Quat& quat);
	void Decompose(const Matrix4& m, Vector3& pos, Quat& rot, Vector3& scale);

	float Distance(const Vector3& a, const Vector3& b);

	const Matrix4& TransformToCameraTransform(const Matrix4& mat);

	// Returns a random number [0..1]
	static float frand()
	{
		//	return ((float)(rand() & 0xffff)/(float)0xffff);
		return (float)rand() / (float)RAND_MAX;
	}
}
