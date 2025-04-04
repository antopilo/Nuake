#pragma once
#include <string>
#include <any>
#include <map>
#include <vector>

#include "Nuake/Core/Logger.h"
#include "Nuake/Core/GameState.h"
#include "Nuake/Core/MulticastDelegate.h"
#include "Nuake/Core/Object/Object.h"

#include "Nuake/Scene/Scene.h"
#include "Nuake/Scene/SceneSystems.h"

#include <entt/entt.hpp>

class ModuleInstance
{
public:
	entt::meta_any instance;
	std::string Name;

	entt::meta_type Resolve()
	{ 
		Nuake::Logger::Log("Resolved!" + Name, "ModuleReflection", Nuake::LOG_TYPE::VERBOSE); \
		return entt::resolve(entt::hashed_string(Name.c_str()));
	} 

	MulticastDelegate<Ref<Nuake::Scene>> OnSceneLoad;
	MulticastDelegate<float> OnUpdate;
	MulticastDelegate<float> OnFixedUpdate;
	MulticastDelegate<Nuake::GameState> OnGameStateChanged;
};

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
		ClassFactory.func<&klass::ClassName>(entt::hashed_string("ClassName"))							\
		.prop(Nuake::HashedName::DisplayName, #klass); \
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
	template<auto F, typename... Args>																					\
	static void BindFunc(const char* funcName, Args... args)                            \
	{																									\
		ClassFactory																				\
		.func<F>(entt::hashed_string(funcName))												\
		.prop(Nuake::HashedName::DisplayName, funcName)	 \
		.prop(Nuake::HashedName::ArgsName, std::vector<std::string>({args...})); \
	} \
	\

#define NUAKEMODULE(moduleName)													\
using TypeNameMap = std::map<entt::id_type, std::string>;						\
																				\
class moduleName : public ModuleInstance										\
{																				\
public:																			\
    std::string Description = "No Description";									\
	entt::meta_factory<moduleName> ModuleFactory = entt::meta<moduleName>();	\
	TypeNameMap TypeNames;														\
																				\
	std::map<entt::id_type, std::vector<std::string>> FuncArgNames;				\
																				\
	inline static auto ClassFactory = entt::meta<moduleName>();					\
	template<auto T, typename... Args>											\
	static void BindFunction(const std::string& name, Args... argNames)			\
	{																			\
		entt::id_type typeId = entt::hashed_string(name.c_str());				\
		ClassFactory.func<T>(typeId)											\
		.prop(Nuake::HashedName::DisplayName, name) 							\
		.prop(Nuake::HashedName::ArgsName, std::vector<const char*>({argNames...}));										\
	}																			\
																				\
	template<auto Func>															\
	static auto RegisterSetting(const char* settingName)						\
	{																			\
		return ClassFactory														\
			.data<Func>(entt::hashed_string(settingName))						\
			.prop(Nuake::HashedName::DisplayName, settingName);					\
	}																			\
																				\
	template<typename T>														\
	void RegisterComponent()													\
	{																			\
		Nuake::SceneSystemDB::Get().RegisterComponent<T>();						\
	}																			\
																				\
	template<typename T>														\
	void RegisterComponentSystem()												\
	{																			\
		Nuake::SceneSystemDB::Get().RegisterSceneSystem<T>();					\
	}																			\
																				\
	template<typename... Args>													\
	auto Invoke(const std::string& funcName, Args... args)						\
	{																			\
		auto reflection = Resolve();											\
		for (auto [id, func] : reflection.func())								\
		{																		\
			const std::string_view returnType = func.ret().info().name();		\
			const std::string_view funcTypeName = GetTypeName(id);				\
			if (funcName == funcTypeName)										\
			{																	\
				return func.invoke(instance, args...);							\
			}																	\
		}																		\
	}																			\
																				\
	std::vector<std::string> GetFuncArgNames(entt::id_type id)					\
	{																			\
		return FuncArgNames[id];												\
	}																			\
																				\
	std::vector<std::string> GetAllFuncNames()									\
	{																			\
		std::vector<std::string> names;											\
		names.reserve(TypeNames.size());										\
		for (const auto& [_, name] : TypeNames)									\
		{																		\
			names.push_back(name);												\
		}																		\
		return names;															\
	}																			\
																				\
		std::string_view GetTypeName(entt::id_type id)							\
		{																		\
			if (TypeNames.contains(id))											\
			{																	\
				return TypeNames[id];											\
			}																	\
			return "Unknown";													\
		}																		\
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
			T* newInstance = new T();
			newInstance->instance = entt::resolve<T>().construct();
			Modules[typeid(T).name()] = (ModuleInstance*)newInstance;
			return *(T*)std::any_cast<ModuleInstance*>(Modules[typeid(T).name()]);
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

			return *(T*)std::any_cast<ModuleInstance*>(Modules[typeName]);
		}

		ModuleInstance& GetBaseImpl(const std::string& moduleName)
		{
			for (auto& [name, _] : Modules)
			{
				if (name == moduleName)
				{
					return *std::any_cast<ModuleInstance*>(Modules[name]);
				}
			}

			assert(false && "Module not found.");
		}

		entt::meta_type GetModuleMeta(const std::string& moduleName)
		{
			for (auto& [name, _] : Modules)
			{
				if (name == moduleName)
				{
					return std::any_cast<ModuleInstance*>(Modules[name])->Resolve();
				}
			}
			
			assert(false && "Module not found");
			return entt::meta_type();
		}

		std::vector<std::string> GetModules()
		{
			std::vector<std::string> moduleNames;
			moduleNames.reserve(Modules.size());

			for(auto& [moduleName, _] : Modules)
			{
				moduleNames.push_back(moduleName);
			}

			return moduleNames;
		}

		json GenerateModuleAPI();

	private:
		std::map<std::string, std::any> Modules;

	};
}