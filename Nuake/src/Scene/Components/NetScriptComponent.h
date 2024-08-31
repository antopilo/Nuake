
#pragma once
#include "src/Core/Core.h"
#include "src/Core/Maths.h"
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
		Prefab,
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

				if (e.Value.has_value() || e.DefaultValue.has_value())
				{
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
					case NetScriptExposedVarType::Prefab:
					{
						if (e.Value.has_value())
						{
							varJ["Value"] = std::any_cast<std::string>(e.Value);
							varJ["DefaultValue"] = "";
						}
						else
						{
							varJ["Value"] = "";
							varJ["DefaultValue"] = "";
						}

						break;
					}
					case NetScriptExposedVarType::Vector2:
					{
						Vector2 value = std::any_cast<Vector2>(e.Value);
						varJ["Value"][0] = value.x;
						varJ["Value"][1] = value.y;

						value = std::any_cast<Vector2>(e.DefaultValue);
						varJ["DefaultValue"][0] = value.x;
						varJ["DefaultValue"][1] = value.y;
						break;
					}
					case NetScriptExposedVarType::Vector3:
					{
						Vector3 value = std::any_cast<Vector3>(e.Value);
						varJ["Value"][0] = value.x;
						varJ["Value"][1] = value.y;
						varJ["Value"][2] = value.z;

						value = std::any_cast<Vector3>(e.DefaultValue);
						varJ["DefaultValue"][0] = value.x;
						varJ["DefaultValue"][1] = value.y;
						varJ["DefaultValue"][2] = value.z;
						break;
					}
					case NetScriptExposedVarType::Entity:
					{
						int value = -1;
						if (e.Value.has_value())
						{
							value = std::any_cast<int>(e.Value);
						}

						varJ["Value"] = value;
						varJ["DefaultValue"] = value;
						break;
					}

					default:
						varJ["Value"] = 0;
						break;
					}

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
					case NetScriptExposedVarType::Prefab:
					{
						exposedVar.Value = (std::string)exposedVarJson["Value"];
						exposedVar.DefaultValue = (std::string)exposedVarJson["DefaultValue"];
						break;
					}
					case NetScriptExposedVarType::String:
						exposedVar.Value = (std::string)exposedVarJson["Value"];
						exposedVar.DefaultValue = (std::string)exposedVarJson["DefaultValue"];
						break;
					case NetScriptExposedVarType::Vector2:
					{
						float x = exposedVarJson["Value"][0];
						float y = exposedVarJson["Value"][1];
						exposedVar.Value = Vector2(x, y);

						x = exposedVarJson["DefaultValue"][0];
						y = exposedVarJson["DefaultValue"][1];
						exposedVar.DefaultValue = Vector2(x, y);
						break;
					}
					case NetScriptExposedVarType::Vector3:
					{
						float x = exposedVarJson["Value"][0];
						float y = exposedVarJson["Value"][1];
						float z = exposedVarJson["Value"][2];
						exposedVar.Value = Vector3(x, y, z);

						x = exposedVarJson["DefaultValue"][0];
						y = exposedVarJson["DefaultValue"][1];
						z = exposedVarJson["DefaultValue"][2];
						exposedVar.DefaultValue = Vector3(x, y, z);
						break;
					}
					case NetScriptExposedVarType::Entity:
					{
						int x = exposedVarJson["Value"];
						exposedVar.Value = x;

						x = exposedVarJson["DefaultValue"];
						exposedVar.DefaultValue = x;
						break;
					}
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