#pragma once
#include "src/Scripting/WrenScript.h"
#include "src/Resource/Serializable.h"

namespace Nuake {
	class WrenScriptComponent
	{
	public:
		std::string Script;

		unsigned int mModule = 0;
		Ref<WrenScript> mWrenScript;

		json Serialize()
		{
			BEGIN_SERIALIZE();
			SERIALIZE_VAL(Script);
			END_SERIALIZE();
		}

		bool Deserialize(std::string str)
		{
			BEGIN_DESERIALIZE();
			if (j.contains("Script"))
				Script = j["Script"];


			return true;
		}
	};
}