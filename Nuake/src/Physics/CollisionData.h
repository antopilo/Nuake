#pragma once
#include <src/Core/Maths.h>

namespace Nuake {

	namespace Physics {

		struct CollisionData
		{
			uint32_t Entity1;
			uint32_t Entity2;
			Vector3 Normal;
			Vector3 Position;
		};
	}
}
