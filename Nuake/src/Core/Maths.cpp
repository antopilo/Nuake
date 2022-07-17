#include "src/Core/Maths.h"

using namespace Nuake;

Quat Nuake::QuatFromEuler(float x, float y, float z)
{
	// Taken from: https://gamedev.stackexchange.com/questions/13436/glm-euler-angles-to-quaternion
	Quat QuatAroundX = Quat(x, Vector3(1.0, 0.0, 0.0));
	Quat QuatAroundY = Quat(y, Vector3(0.0, 1.0, 0.0));
	Quat QuatAroundZ = Quat(z, Vector3(0.0, 0.0, 1.0));
	return QuatAroundX * QuatAroundY * QuatAroundZ;
}
