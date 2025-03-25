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


void ExampleModuleLog(const std::string& hi)
{
	Nuake::Logger::Log(hi, "ExampleModule", Nuake::VERBOSE);
}

NUAKEMODULE(ExampleModule)
void ExampleModule_Startup()
{
	using namespace Nuake;

	TestClass::InternalInitializeClass();


	auto& module = ModuleDB::Get().RegisterModule<ExampleModule>();
	module.instance = module.Resolve().construct();

	module.Description = "This is an example module";

	// This is to expose functions to the rest of the engine
	module.BindFunction<ExampleFunction>("ExampleFunction");
	module.BindFunction<ExampleModuleLog>("ExampleModuleLog", "hi");
	//module.Invoke("ExampleModuleLog", "Hello World");

	entt::id_type typeId = entt::hashed_string("ExampleModuleLog"); 
	entt::meta_type metaType = entt::resolve(typeId);
	auto resolvedFunc = metaType.func(typeId);

	if (resolvedFunc)
	{
		resolvedFunc.invoke(module.instance, std::string("Hello has been invoked!"));
	}


	Logger::Log("ExampleModule exposed API:", "Module", VERBOSE);

	auto reflection = module.Resolve();
	for (auto [id, func] : reflection.func())
	{
		const std::string_view returnType = func.ret().info().name();
		const std::string_view funcName = module.GetTypeName(id);
		
		if (funcName == "ExampleModuleLog")
		{
			func.invoke(module.instance, "Hi");
		}
		std::string msg = std::string(returnType) + " " + std::string(funcName) + "(";
		auto argNames = module.GetFuncArgNames(id);
		std::vector<std::string_view> args;
		for (int i = 0; i < func.arity(); i++)
		{
			const std::string argType = std::string(func.arg(i).info().name());
			args.push_back(argType);
	
			msg += argType + " " + argNames[i];

			if (i < func.arity() - 1)
			{
				msg += ", ";
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