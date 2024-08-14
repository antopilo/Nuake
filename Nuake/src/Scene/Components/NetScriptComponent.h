#pragma once
#include "src/Core/Core.h"
#include "src/Core/FileSystem.h"
#include "src/Core/Logger.h"
#include "src/Resource/Serializable.h"
#include "src/Resource/File.h"
#include <any>

namespace Nuake {

	enum class NetScriptExposedVarType
	{
		Bool,
		Float,
		Double,
		String,
		Entity,
		Vector2,
		Vector3,
		Vector4
	};

	struct NetScriptExposedVar
	{
		std::string Name;
		std::any Value;
		std::any DefaultValue;
		NetScriptExposedVarType Type;
	};

	class NetScriptComponent
	{
	public:
		std::string ScriptPath;
		std::string Class;

		bool Initialized = false;

		std::vector<NetScriptExposedVar> ExposedVar;

		json Serialize()
		{
			BEGIN_SERIALIZE();
			SERIALIZE_VAL(ScriptPath);
			SERIALIZE_VAL(Class);

			json exposedVarJson;
			int i = 0;
			for (auto& e : ExposedVar)
			{
				json varJ;
				varJ["Name"] = e.Name;
				varJ["Type"] = e.Type;

				switch (e.Type)
				{
					case NetScriptExposedVarType::Bool:
						varJ["Value"] = std::any_cast<bool>(e.Value);
						varJ["DefaultValue"] = std::any_cast<bool>(e.DefaultValue);
						break;
					case NetScriptExposedVarType::Float:
						varJ["Value"] = std::any_cast<float>(e.Value);
						varJ["DefaultValue"] = std::any_cast<float>(e.DefaultValue);
						break;
					case NetScriptExposedVarType::Double:
						varJ["Value"] = std::any_cast<double>(e.Value);
						varJ["DefaultValue"] = std::any_cast<double>(e.DefaultValue);
						break;
					case NetScriptExposedVarType::String:
						varJ["Value"] = std::any_cast<std::string>(e.Value);
						varJ["DefaultValue"] = std::any_cast<std::string>(e.DefaultValue);
						break;
					default:
						varJ["Value"] = 0;
						break;
				}

				exposedVarJson[i] = varJ;
				i++;
			}

			j["ExposedVar"] = exposedVarJson;

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

			if (j.contains("ExposedVar"))
			{
				for (auto& exposedVarJson : j["ExposedVar"])
				{
					NetScriptExposedVar exposedVar;
					exposedVar.Name = exposedVarJson["Name"];
					exposedVar.Type = exposedVarJson["Type"];
					switch (exposedVar.Type)
					{
					case NetScriptExposedVarType::Bool:
						exposedVar.Value = (bool)exposedVarJson["Value"];
						exposedVar.DefaultValue = (bool)exposedVarJson["DefaultValue"];
						break;
					case NetScriptExposedVarType::Float:
						exposedVar.Value = (float)exposedVarJson["Value"];
						exposedVar.DefaultValue = (float)exposedVarJson["DefaultValue"];
						break;
					case NetScriptExposedVarType::Double:
						exposedVar.Value = (double)exposedVarJson["Value"];
						exposedVar.DefaultValue = (double)exposedVarJson["DefaultValue"];
						break;
					case NetScriptExposedVarType::String:
						exposedVar.Value = (std::string)exposedVarJson["Value"];
						exposedVar.DefaultValue = (std::string)exposedVarJson["DefaultValue"];
						break;
					default:
						exposedVar.DefaultValue = 0;
						break;
					}

					ExposedVar.push_back(exposedVar);
				}
			}

			return true;
		}
	};
}