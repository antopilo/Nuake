#pragma once

#include "../Entity.h"


struct ParentComponent
{
	Entity Parent;
	bool HasParent = false;
	std::vector<Entity> Children = std::vector<Entity>();
};