#pragma once
#include "src/Core/Core.h"
#include "src/Core/FileSystem.h"
#include "src/Core/Logger.h"
#include "src/Resource/Serializable.h"
#include "src/Resource/File.h"

namespace Nuake {

	class NetScriptComponent
	{
	public:
		std::string ScriptPath;
		std::string Class;

		bool Initialized = false;

		json Serialize()
		{
			BEGIN_SERIALIZE();
			SERIALIZE_VAL(ScriptPath);
			SERIALIZE_VAL(Class);
			END_SERIALIZE();
		}

		bool Deserialize(const json& j)
		{
			if (j.contains("ScriptPath"))
			{
				ScriptPath = j["ScriptPath"];
			}

			if (j.contains("Class"))
			{
				Class = j["Class"];
			}

			return true;
		}
	};
}