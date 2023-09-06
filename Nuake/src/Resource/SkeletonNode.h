#pragma once
#include "src/Core/Core.h"
#include "src/Core/Maths.h"

namespace Nuake
{
	struct SkeletonNode
	{
		Matrix4 Transform;
		std::string Name;
		int ChildrenCount;
		std::vector<SkeletonNode> Children;
	};
}