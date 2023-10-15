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
		m_HostInstance = new Coral::HostInstance();
		m_HostInstance->Initialize(settings);

		
	}

	ScriptingEngineNet::~ScriptingEngineNet()
	{
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
		auto loadContext = m_HostInstance->CreateAssemblyLoadContext("NuakeEngineContext");

		auto& assembly = loadContext.LoadAssembly("NuakeNet.dll");
		assembly.AddInternalCall("Nuake.Net.Engine", "LoggerLogIcall", reinterpret_cast<void*>(&Log));
		assembly.UploadInternalCalls();

		auto& engineType = assembly.GetType("Nuake.Net.Engine");
		auto engineInstance = engineType.CreateInstance();

		Coral::NativeString param1 = Coral::NativeString::FromUTF8("Hello from CPP");;
		engineInstance.InvokeMethod("Log", std::string("Hello from CPP"));

		engineInstance.Destroy();

		Coral::GC::Collect();
		Coral::GC::WaitForPendingFinalizers();

		m_HostInstance->UnloadAssemblyLoadContext(loadContext);
	}
}