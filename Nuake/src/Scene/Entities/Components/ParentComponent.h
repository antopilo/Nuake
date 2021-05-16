#pragma once

#include "../Entity.h"

struct ParentComponent
{
	Entity Parent;
	bool HasParent = false;
	std::vector<Entity> Children = std::vector<Entity>();

	json Serialize()
	{
		BEGIN_SERIALIZE();
		SERIALIZE_VAL(HasParent);
		if(HasParent)
			SERIALIZE_VAL_LBL("Parent", Parent.GetHandle());
		END_SERIALIZE();
	}
};