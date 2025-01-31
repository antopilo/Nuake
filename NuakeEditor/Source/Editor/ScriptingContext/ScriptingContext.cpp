#include "ScriptingContext.h"
#include "Nuake/Core/String.h"

#include "Modules/EditorModule.h"

void ScriptingContext::Initialize()
{
	modules =
	{
		CreateRef<EditorNetAPI>()
	};

	for (auto& m : modules)
	{
		m->RegisterMethods();
	}

	// Load Nuake assembly DLL
	/*auto loadContext2 = Nuake::ScriptingEngineNet::Get().GetHostInstance()->CreateAssemblyLoadContext("NuakeEditorContext");
	nuakeAssembly = Nuake::ScriptingEngineNet::Get().ReloadEngineAPI(loadContext2);
	m_EditorAssembly = loadContext2.LoadAssembly("EditorNet.dll");

	for (const auto& netModule : modules)
	{
		for (const auto& [methodName, methodPtr] : netModule->GetMethods())
		{
			auto namespaceClassSplit = Nuake::String::Split(methodName, '.');
			m_EditorAssembly.AddInternalCall("Nuake." + namespaceClassSplit[0], namespaceClassSplit[1], methodPtr);
		}
	}

	m_EditorAssembly.UploadInternalCalls();
	m_EditorAssembly.GetType("Nuake.Editor").InvokeStaticMethod("Initialize");*/
}

void ScriptingContext::PushCommand(const std::string & command, const std::string & arg)
{
	m_EditorAssembly.GetType("Nuake.Editor").InvokeStaticMethod("Initialize");
}
