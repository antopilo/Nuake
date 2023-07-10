#include "src/Core/Maths.h"

using namespace Nuake;

Quat Nuake::QuatFromEuler(float x, float y, float z)
{
	return Quat(Vector3(Rad(z), Rad(y), Rad(x)));
}
