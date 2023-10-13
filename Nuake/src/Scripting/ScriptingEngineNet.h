#pragma once
#include "src/Core/Core.h"

namespace Coral
{
	class HostInstance;
	class AssemblyLoadContext;
}

namespace Nuake
{
	class ScriptingEngineNet
	{
	private:
		Scope<Coral::HostInstance> m_HostInstance;
		Scope<Coral::AssemblyLoadContext> m_LoadContext;

		ScriptingEngineNet();
		~ScriptingEngineNet();

	public:
		static ScriptingEngineNet& Get();

		void Initialize();
	};
}