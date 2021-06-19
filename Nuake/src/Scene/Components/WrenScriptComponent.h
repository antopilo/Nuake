#pragma once
#include "../Scripting/WrenScript.h"
#include <src/Resource/Serializable.h>

class WrenScriptComponent
{
public:
	std::string Script;
	std::string Class;

	Ref<WrenScript> WrenScript;

	json Serialize()
	{
		BEGIN_SERIALIZE();
		SERIALIZE_VAL(Script);
		SERIALIZE_VAL(Class);
		END_SERIALIZE();
	}

	bool Deserialize(std::string str)
	{
		BEGIN_DESERIALIZE();
		if (j.contains("Script"))
			Script = j["Script"];
		if (j.contains("Class"))
			Class = j["Class"];
		

		return true;
	}
};