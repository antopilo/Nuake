#include "src/Core/Maths.h"

using namespace Nuake;

Quat Nuake::QuatFromEuler(float x, float y, float z)
{
	return Quat(Vector3(Rad(z), Rad(y), Rad(x)));
}

Vector3 Nuake::QuatToDirection(const Quat& quat)
{
	//return quat * Vector3(0, 0, -1);
	return glm::rotate(glm::inverse(quat), glm::vec3(0.0, 0.0, 1.0));
}