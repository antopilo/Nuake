#include "ScriptingContext.h"
#include "src/Core/String.h"

#include "Modules/EditorModule.h"

void ScriptingContext::Initialize()
{
	m_Modules =
	{
		CreateRef<EditorNetAPI>()
	};

	for (auto& m : m_Modules)
	{
		m->RegisterMethods();
	}

	// Load Nuake assembly DLL
	/*auto m_LoadContext2 = Nuake::ScriptingEngineNet::Get().GetHostInstance()->CreateAssemblyLoadContext("NuakeEditorContext");
	m_NuakeAssembly = Nuake::ScriptingEngineNet::Get().ReloadEngineAPI(m_LoadContext2);
	m_EditorAssembly = m_LoadContext2.LoadAssembly("EditorNet.dll");

	for (const auto& netModule : m_Modules)
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
