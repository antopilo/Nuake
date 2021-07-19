#include "VM.h"

namespace Nuake {
	VM::VM()
	{
		WrenConfiguration config;
        wrenInitConfiguration(&config);

        //config.loadModuleFn = &myLoadModule;
        //config.errorFn = &errorFn;
        //config.writeFn = &writeFn;
        //config.bindForeignMethodFn = &bindForeignMethod;

        m_VM = wrenNewVM(&config);
	}
}