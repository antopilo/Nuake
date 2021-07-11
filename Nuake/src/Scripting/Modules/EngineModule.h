#pragma once
#include "wren.h"
#include <string>
#include <src/Core/Logger.h>
#include "ScriptModule.h"
#include <iostream>
#include <wren.h>
namespace Nuake
{
	namespace ScriptAPI
	{
		class EngineModule : public ScriptModule
		{
			std::string ModuleName = "Engine";

			std::string GetModuleName() override
			{
				return "Engine";
			}

			void RegisterModule(WrenVM* vm) override
			{
				RegisterMethod("Log(_)", (void*)Log);
			}

			static void Log(WrenVM* vm)
			{
				std::string msg = wrenGetSlotString(vm, 1);
				Logger::Log(msg);
			}
		};
	}
}