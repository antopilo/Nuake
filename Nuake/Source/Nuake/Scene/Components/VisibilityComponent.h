#pragma once
#include "Nuake/Resource/Serializable.h"

struct VisibilityComponent
{
	bool Visible = true;

	json Serialize()
	{
		BEGIN_SERIALIZE();
		SERIALIZE_VAL(Visible);
		END_SERIALIZE();
	}

	bool Deserialize(const json& j)
	{
		if (j.contains("Visible"))
		{
			Visible = j["Visible"];
		}

		return true;
	}
};