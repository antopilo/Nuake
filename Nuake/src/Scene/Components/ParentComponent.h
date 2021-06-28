#pragma once

#include "../Entities/Entity.h"

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

		//int i = 0;
		//for (auto& c : Children) {
		//	j["Children"][0] = c.GetHandle();
		//	i++;
		//}
		
		END_SERIALIZE();
	}

	bool Deserialize(std::string str)
	{
		BEGIN_DESERIALIZE();

		//this->Parent = Entity{ j["Parent"], Engine::GetCurrentScene().get() };
		
		return true;

	}
};