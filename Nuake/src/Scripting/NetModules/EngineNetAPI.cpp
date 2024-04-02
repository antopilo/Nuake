#include "EngineNetAPI.h"

namespace Nuake {

	void Log(Coral::String string)
	{
		Logger::Log(string, ".net", VERBOSE);
	}

	void EngineNetAPI::RegisterMethods()
	{
		RegisterMethod("Engine.LoggerLogIcall", (void*)(&Log));
	}

}
