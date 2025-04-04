#include "ExampleModule.h"

#include "Nuake/Modules/ModuleDB.h"

#include "Nuake/Core/Logger.h"

#include "Nuake/Resource/Serializer/ComponentSerializer.h"
#include "Nuake/Scene/Components/AudioEmitterComponent.h"
#include "Nuake/Scene/Components/LightComponent.h"
#include "Nuake/Scene/Components/Component.h"

#include <entt/entt.hpp>
#include "Thirdparty/entt/src/entt/meta/meta.hpp"
#include "Thirdparty/entt/src/entt/core/hashed_string.hpp"


namespace Nuake
{
	class MyModuleComponent : public Component
	{
		NUAKECOMPONENT(MyModuleComponent, "My custom component")

		static void InitializeComponentClass()
		{
			BindComponentField<&MyModuleComponent::MyFloat>("MyFloat", "My Float");
		}

	public:
		float MyFloat = 1.0f;
	};
}

void ExampleFunction() { }

class TestClass : public Class
{
	NUAKECLASS(TestClass)
	static void InitializeClass()
	{
		BindField<&TestClass::Intensity>("Intensity", "Intensity");
		BindProperty<&TestClass::SetIntensity, &TestClass::GetIntensity>("Intensity2", "Intensity2");
		BindFunc<&TestClass::SetIntensity>("SetIntensity", "intensity");
	}

public:
	float Intensity = 1.0f;

	void SetIntensity(float value)
	{
		Intensity = value;
	}

	float GetIntensity()
	{
		return Intensity;
	}
};


void ExampleModuleLog(NativeString hi)
{
	Nuake::Logger::Log(hi, "ExampleModule", Nuake::VERBOSE);
}

float mySetting = 8.0f;

NUAKEMODULE(ExampleModule)
void ExampleModule_Startup()
{
	using namespace Nuake;

	TestClass::InternalInitializeClass();

	// Register the module & info
	auto& module = ModuleDB::Get().RegisterModule<ExampleModule>();
	module.instance = module.Resolve().construct();
	module.Description = "This is an example module";

	// This is to expose parameters in the modules settings
	module.RegisterSetting<&mySetting>("mySetting");

	// This is to expose functions to the rest of the engine
	module.BindFunction<ExampleFunction>("ExampleFunction");
	module.BindFunction<ExampleModuleLog>("ExampleModuleLog16", "hi2");

	// The module can hook to certain events
	module.OnUpdate.AddStatic([](float ts)
	{

	});

	module.OnFixedUpdate.AddStatic([](float ts) 
	{
		
	});



	entt::id_type typeId = entt::hashed_string(TestClass::ClassName().c_str());
	entt::meta_type metaType = entt::resolve(typeId);
	auto resolvedFunc = metaType.func(typeId);

	Logger::Log("ExampleModule exposed API:", "Module", VERBOSE);

	auto reflection = metaType;
	for (auto [id, func] : reflection.func())
	{
		const std::string_view returnType = func.ret().info().name();
		auto propDisplayName = func.prop(HashedName::DisplayName);
		std::string funcName = "UnknownFunc";
		if (propDisplayName)
		{
			funcName = std::string(*propDisplayName.value().try_cast<const char*>());
		}
		
		std::string msg = std::string(returnType) + " " + std::string(funcName) + "(";
		std::vector<std::string_view> args;

		if (func.arity() > 0)
		{
			auto propArgsName = func.prop(HashedName::ArgsName);
			if (propArgsName)
			{
				std::vector<std::string> argsName = *propArgsName.value().try_cast<std::vector<std::string>>();
				for (int i = 0; i < func.arity(); i++)
				{
					const std::string argType = std::string(func.arg(i).info().name());
					args.push_back(argType);

					msg += argType + " " + argsName[i];

					if (i < func.arity() - 1)
					{
						msg += ", ";
					}
				}
			}
		}

		
		msg += ")";
	
		Logger::Log(msg, "", VERBOSE);
	}

	// This is where you would initialize your module
	// This function is called when the engine starts up

	// You can register custom Scene Systems here
	// Example: See Audio Module for reference

	// You can register custom asset bakers to support custom formats
	// Example: See Assimp Module for reference
	using namespace Nuake;
	MyModuleComponent::InternalInitializeClass();

	AudioEmitterComponent component;
	LightComponent lightComponent;

	ComponentSerializer serializer;

	json jsonData = serializer.Serialize(component);
	json jsonDataOG = component.Serialize();
	//Logger::Log("Serialized reflected component: " + jsonData.dump(4), "ExampleModule", VERBOSE);
	//Logger::Log("Serialized original component: " + jsonDataOG.dump(4), "ExampleModule", VERBOSE);
	
	jsonData = serializer.Serialize(lightComponent);
	jsonDataOG = lightComponent.Serialize();
	//Logger::Log("Serialized reflected component: " + jsonData.dump(4), "ExampleModule", VERBOSE);
	//Logger::Log("Serialized original component: " + jsonDataOG.dump(4), "ExampleModule", VERBOSE);
}

void ExampleModule_Shutdown()
{

}