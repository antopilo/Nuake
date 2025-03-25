#pragma once
#include <string>
#include <any>
#include <vector>

#include "Nuake/Core/Object/Object.h"

#include <entt/entt.hpp>

class Class
{
protected:
	static void InitializeClass() {}

	static void (*GetInitializeClass(void))()
	{
		return &Class::InitializeClass;
	}

public:
	static std::string GetName(const entt::meta_type& componentMeta);
};

#define NUAKECLASS(klass)																				\
public:																									\
    static std::string ClassName()																		\
    {																									\
        static std::string className = #klass;															\
        return className;																				\
    }																									\
																										\
    static void (*GetInitializeClass())()														\
    {																									\
        return &klass::InitializeClass;														\
    }																									\
																										\
	inline static auto ClassFactory = entt::meta<klass>();												\
    static void InternalInitializeClass()																\
    {																									\
        static bool initialized = false;																\
        if (initialized)																				\
            return;																						\
																										\
		ClassFactory.type(entt::hashed_string(#klass));													\
		ClassFactory.func<&klass::ClassName>(entt::hashed_string("ClassName"));							\
		if (klass::GetInitializeClass() != Class::GetInitializeClass())			\
        {																								\
            GetInitializeClass()();															\
        }																								\
																										\
		initialized = true;																				\
	}																									\
																										\
	template<auto Data>																					\
    static auto BindField(const char* varName, const char* displayName)									\
    {																									\
        return ClassFactory																			\
            .data<Data>(entt::hashed_string(varName))													\
            .prop(Nuake::HashedName::DisplayName, displayName);												\
    }																									\
																										\
    template<auto Getter, auto Setter>																	\
    static auto BindProperty(const char* varName, const char* displayName)								\
    {																									\
        return ClassFactory																			\
            .data<Getter, Setter>(entt::hashed_string(varName))											\
            .prop(Nuake::HashedName::DisplayName, displayName);												\
    }																									\
																										\
	template<auto Func>																					\
		static void BindAction(const char* funcName, const char* actionName)                            \
	{																									\
		ClassFactory																				\
		.func<Func>(entt::hashed_string(funcName))														\
		.prop(Nuake::HashedName::DisplayName, actionName);													\
	}


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