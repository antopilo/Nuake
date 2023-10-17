#include "EngineNetAPI.h"

namespace Nuake {

	void Log(Coral::NativeString string)
	{
		Logger::Log(string.ToString(), ".net", VERBOSE);
	}

	void EngineNetAPI::RegisterMethods()
	{
		RegisterMethod("Engine.LoggerLogIcall", (void*)(&Log));
	}

}
