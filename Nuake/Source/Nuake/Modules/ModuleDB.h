#pragma once
#include <string>
#include <any>
#include <vector>

#include <entt/entt.hpp>

#define NUAKEMODULE(moduleName)									\
using TypeNameMap = std::map<entt::id_type, std::string>;		\
																\
struct moduleName												\
{																\
	entt::meta_any instance;									\
    const std::string Name = #moduleName;						\
    std::string Description = "No Description";					\
	entt::meta_factory<moduleName> ModuleFactory = entt::meta<moduleName>();						\
	TypeNameMap TypeNames;										\
																\
	std::map<entt::id_type, std::vector<std::string>> FuncArgNames; \
\
	template<auto T, typename... Args>										\
	inline moduleName& BindFunction(const std::string& name, Args... argNames)		\
	{															\
		entt::id_type typeId = entt::hashed_string(name.c_str()); \
		TypeNames[typeId] = name; \
		FuncArgNames[typeId] = { argNames ... }; \
		ModuleFactory.func<T>(typeId);	\
		return *this; \
	}															\
																\
	auto Resolve()												\
	{															\
		return entt::resolve<moduleName>();							\
	}															\
\
	template<typename... Args> \
	auto Invoke(const std::string& funcName, Args... args) \
	{ \
		auto reflection = Resolve();\
		for (auto [id, func] : reflection.func())\
		{\
			const std::string_view returnType = func.ret().info().name();\
			const std::string_view funcTypeName = GetTypeName(id);\
			if (funcName == funcTypeName)\
			{\
				return func.invoke(instance, args...);\
			}\
		} \
	} \
\
std::vector<std::string> GetFuncArgNames(entt::id_type id) \
{ \
	return FuncArgNames[id]; \
} \
\
std::vector<std::string> GetAllFuncNames() \
{\
	std::vector<std::string> names; \
	names.reserve(TypeNames.size()); \
	for (const auto& [_, name] : TypeNames) \
	{ \
		names.push_back(name);\
	}\
	return names;\
}\
\
	std::string_view GetTypeName(entt::id_type id) \
	{ \
		if (TypeNames.contains(id)) \
		{ \
			return TypeNames[id]; \
		} \
		return "Unknown"; \
	}\
};

namespace Nuake
{
	// This is a singleton that knows about every modules and their public API
	class ModuleDB
	{
	public:
		ModuleDB() = default;
		~ModuleDB() = default;

		static ModuleDB& Get()
		{
			static ModuleDB instance;
			return instance;
		}

		template<typename T>
		T& RegisterModule()
		{
			Modules[typeid(T).name()] = T();
			return std::any_cast<T&>(Modules[typeid(T).name()]);
		}

		template<typename T>
		T& GetModule()
		{
			const std::string_view typeName = typeid(T).name();
			if (!Modules.contains(typeName))
			{
				assert(false && "Module not found.");
				return;
			}

			return std::any_cast<T>(Modules[typeName]);
		}

	private:
		std::map<std::string, std::any> Modules;

	};
}