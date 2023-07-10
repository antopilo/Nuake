#include "src/Core/Maths.h"

using namespace Nuake;

Quat Nuake::QuatFromEuler(float x, float y, float z)
{
	return Quat(Vector3(Rad(x), Rad(y), Rad(z)));
}

Vector3 Nuake::QuatToDirection(const Quat& quat)
{
	return quat * Vector3(0, 0, -1);
}