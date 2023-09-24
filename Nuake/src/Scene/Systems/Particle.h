#pragma once
#include "src/Core/Maths.h"

namespace Nuake
{
	struct Particle
	{
		Vector3 Position;
		Vector3 Velocity;
		float Scale;
		Color Color;
		float Life;
	};
}
