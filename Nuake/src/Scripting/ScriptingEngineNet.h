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
		Coral::HostInstance* m_HostInstance;
		Coral::AssemblyLoadContext* m_LoadContext;

		ScriptingEngineNet();
		~ScriptingEngineNet();

	public:
		static ScriptingEngineNet& Get();

		void Initialize();
	};
}