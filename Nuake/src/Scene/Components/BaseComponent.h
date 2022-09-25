#pragma once
#include "src/Resource/Serializable.h"

struct VisibilityComponent
{
	bool Visible = true;

	json Serialize()
	{
		BEGIN_SERIALIZE();
		SERIALIZE_VAL(Visible);
		END_SERIALIZE();
	}

	bool Deserialize(std::string str)
	{
		BEGIN_DESERIALIZE();
		
		if (j.contains("Visible"))
		{
			Visible = j["Visible"];
		}

		return true;
	}
};