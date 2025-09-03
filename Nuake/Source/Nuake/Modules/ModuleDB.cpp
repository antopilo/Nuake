#include "ModuleDB.h"

using namespace Nuake;

json ModuleDB::GenerateModuleAPI()
{
	json moduleAPI;

	int i = 0;
	for (auto& [name, module] : Modules)
	{
		json moduleData;
		moduleData["Name"] = name;

		auto meta = entt::resolve(entt::hashed_string(("class " + name).c_str()));

		int j = 0;
		for (auto [id, func] : meta.func())
		{
			json funcData;

			auto funcName = func.prop(HashedName::DisplayName);
			funcData["Name"] = funcName.value().try_cast<std::string>()->c_str();
			funcData["ReturnType"] = func.ret().info().name();
			funcData["NumArgs"] = func.arity();

			if (func.arity() > 0)
			{
				auto propArgsName = func.prop(HashedName::ArgsName);
				if (propArgsName)
				{
					json argsData;
					std::vector<const char*> argsName = *propArgsName.value().try_cast<std::vector<const char*>>();
					for (int k = 0; k < func.arity(); k++)
					{
						std::string argType = std::string(func.arg(k).info().name());

						if (argType == "class Coral::String" || argType == "class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >")
						{
							argType = "string";
						}

						json argData;
						argData["Name"] = argsName[k];
						argData["Type"] = argType;

						argsData[k] = argData;
					}

					funcData["Args"] = argsData;
				}
			}

			moduleData["Functions"][j] = funcData;
			j++;
		}

		moduleAPI["Modules"][i] = moduleData;

		i++;
	}

	return moduleAPI;
}