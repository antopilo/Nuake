#pragma once
#include "../Entities/Entity.h"

namespace Nuake
{
	struct ParentComponent
	{
		int ParentID;
		Entity Parent;
		bool HasParent = false;
		std::vector<Entity> Children = std::vector<Entity>();

		bool RemoveChildren(Entity ent)
		{
			for (int i = 0; i < Children.size(); i++)
			{
				if (Children[i].GetHandle() == ent.GetHandle())
				{
					Children.erase(Children.begin() + i);
					return true;
				}
			}

			return false;
		}

		json Serialize()
		{
			BEGIN_SERIALIZE();
			SERIALIZE_VAL(HasParent);
			if (HasParent)
				SERIALIZE_VAL_LBL("ParentID", Parent.GetID());

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
			this->HasParent = j["HasParent"];
			if (HasParent)
			{
				this->ParentID = j["ParentID"];
				//this->Parent = Entity{ j["Parent"], Engine::GetCurrentScene().get() };
			}
			return true;
		}
	};
}
