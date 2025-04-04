#include "ModulesAPI.h"

#include "Nuake/Modules/ModuleDB.h"

#include <Coral/Array.hpp>
#include <Coral/String.hpp>

namespace Nuake 
{
	void SetVolume(float vol)
	{
		float myVolume = vol;
		return;
	}

	void ModulesAPI::RegisterMethods()
	{
		isModule = true;

		RegisterMethod("Internals.SetVolumeICall", &SetVolume);

		//const std::string& internalPrefix = "Internal";
		//
		//for (auto& name : ModuleDB::Get().GetModules())
		//{
		//	const std::string& modulePrefix = internalPrefix + ".";
		//	auto meta = entt::resolve(entt::hashed_string(("class " + name).c_str()));
		//	auto instance = ModuleDB::Get().GetBaseImpl(name).instance;
		//	for (auto [id, func] : meta.func())
		//	{
		//		auto funcName = func.prop(HashedName::DisplayName);
		//		const std::string funcNameStd = funcName.value().try_cast<std::string>()->c_str();
		//		const std::string& fullName = modulePrefix + funcNameStd;
		//		
		//		auto funcPtrMeta = func.prop(HashedUserValue::FuncPtr);
		//		auto funcPtr = funcPtrMeta.value().try_cast<void*>();
		//		RegisterMethod(fullName, &SetVolume);
		//	}
		//}
	}

}

