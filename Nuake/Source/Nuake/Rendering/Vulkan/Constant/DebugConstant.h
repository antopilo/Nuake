#pragma once
#include "Nuake/Core/Maths.h"

namespace Nuake
{
	struct DebugConstant
	{
		Vector4 Color = Vector4(1, 1, 1, 1);
		Matrix4 Transform;
		int TextureID;
	};
}