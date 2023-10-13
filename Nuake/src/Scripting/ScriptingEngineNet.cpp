#include "ScriptingEngineNet.h"

#include "src/Core/Logger.h"
#include "src/Core/FileSystem.h"


#include <Coral/HostInstance.hpp>
#include <Coral/GC.hpp>
#include <Coral/NativeArray.hpp>
#include <Coral/Attribute.hpp>


void ExceptionCallback(std::string_view InMessage)
{
	const std::string message = std::string("Unhandled native exception: ") + std::string(InMessage);
	Nuake::Logger::Log(message, ".net", Nuake::CRITICAL);
}

namespace Nuake
{
	ScriptingEngineNet::ScriptingEngineNet()
	{
		auto coralDir = "";
		Coral::HostSettings settings =
		{
			.CoralDirectory = coralDir,
			.ExceptionCallback = ExceptionCallback
		};
		m_HostInstance = CreateScope<Coral::HostInstance>();
		m_HostInstance->Initialize(settings);

		m_LoadContext = CreateScope<Coral::AssemblyLoadContext>(m_HostInstance->CreateAssemblyLoadContext("NuakeEngineContext"));
	}

	ScriptingEngineNet::~ScriptingEngineNet()
	{
		m_HostInstance->UnloadAssemblyLoadContext(*m_LoadContext);
		m_HostInstance->Shutdown();
	}

	ScriptingEngineNet& ScriptingEngineNet::Get()
	{
		static ScriptingEngineNet instance;
		return instance;
	}

	void Log(Coral::NativeString string)
	{
		Logger::Log(string.ToString(), ".net", VERBOSE);
	}

	void ScriptingEngineNet::Initialize()
	{
		
		auto& assembly = m_LoadContext->LoadAssembly("NuakeNet.dll");

		assembly.AddInternalCall("Nuake.Net.Engine", "LoggerLogIcall", reinterpret_cast<void*>(&Log));
		assembly.UploadInternalCalls();

		auto& engineType = assembly.GetType("Nuake.Net.Engine");
		auto engineInstance = engineType.CreateInstance();

		Coral::NativeString param1 = Coral::NativeString::FromUTF8("Hello from CPP");;
		engineInstance.InvokeMethod("Log", param1);

		engineInstance.Destroy();
		Coral::GC::Collect();
	}
}