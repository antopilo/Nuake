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
		int32_t Id = -1;
	};
}