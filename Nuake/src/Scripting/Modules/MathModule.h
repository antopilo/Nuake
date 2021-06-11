#pragma once
#include "wren.h"
#include <string>
#include <src/Core/Logger.h>
#include "ScriptModule.h"
#include <iostream>
#include <wren.h>
#include "../Core/Maths.h"

namespace ScriptAPI
{
	class MathModule : public ScriptModule
	{
		std::string ModuleName = "Math";


		std::string GetModuleName() override
		{
			return "Math";
		}

		void RegisterModule(WrenVM* vm) override
		{
			RegisterMethod("Sqrt_(_,_,_)", (void*)Sqrt);
			
		}

		static void Sqrt(WrenVM* vm)
		{
			float x = wrenGetSlotDouble(vm, 0);
			float y = wrenGetSlotDouble(vm, 0);
			float z = wrenGetSlotDouble(vm, 0);
			float result = glm::sqrt((x * x) + (y * y) + (z * z));
			wrenSetSlotDouble(vm, 0, result);
		}
	};


}