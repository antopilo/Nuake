#pragma once
#include "wren.h"
#include <string>
#include "Nuake/Core/Logger.h>
#include "../Core/Maths.h"
#include "ScriptModule.h"
#include <iostream>
#include <wren.h>
#include "Nuake/Physics/PhysicsManager.h"

namespace Nuake 
{
	namespace ScriptAPI 
	{
		class PhysicsModule : public ScriptModule
		{
			std::string ModuleName = "Engine";

			std::string GetModuleName() override
			{
				return "Physics";
			}

			void RegisterModule(WrenVM* vm) override
			{
				RegisterMethod("Raycast_(_,_,_,_,_,_)", (void*)Raycast);
			}

			static void Raycast(WrenVM* vm)
			{
				
			}
		};
	}
}