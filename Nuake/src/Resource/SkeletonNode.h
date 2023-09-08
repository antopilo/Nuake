#pragma once
#include "src/Core/Core.h"
#include "src/Core/Maths.h"

namespace Nuake
{
	struct SkeletonNode
	{
		Matrix4 Transform;
		Matrix4 Offset;
		std::string Name;
		int ChildrenCount;
		std::vector<SkeletonNode> Children;
		Matrix4 FinalTransform = Matrix4(1.0f);
		int32_t Id = -1;
	};
}