#pragma once
#include "src/Scripting/NetModules/NetAPIModule.h"

namespace Coral
{
	class HostInstance;
	class AssemblyLoadContext;
	class ManagedAssembly;
}

#include "src/Scripting/ScriptingEngineNet.h"

class ScriptingContext
{
private:
	Coral::HostInstance* m_HostInstance;
	Coral::AssemblyLoadContext m_LoadContext;

	std::unordered_map<std::string, Coral::AssemblyLoadContext*> m_LoadedAssemblies;
	std::vector<Ref<Nuake::NetAPIModule>> m_Modules;

	Coral::ManagedAssembly m_NuakeAssembly; // Nuake DLL
	Coral::ManagedAssembly m_EditorAssembly; // Editor DLL
public:
	static ScriptingContext& Get()
	{
		static ScriptingContext instance;
		return instance;
	}

	void Initialize();

	void PushCommand(const std::string& command, const std::string& arg);
};