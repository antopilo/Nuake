#pragma once
#include <src/Vendors/wren/src/include/wren.h>

namespace Nuake {
	class VM 
	{
		VM();

		inline WrenVM* GetVM();
	private:
		WrenVM* m_VM;
	};
}