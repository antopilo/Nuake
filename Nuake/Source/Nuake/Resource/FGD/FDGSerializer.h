#pragma once
#include "FGDClass.h"
#include <string>

namespace Nuake {
	class FGDSerializer
	{
	public:
		static bool BeginFGDFile(const std::string path);

		static bool SerializeClass(FGDClass fgdClass);
		static bool SerializePoint(FGDPointEntity fgdPoint);
		static bool SerializeBrush(FGDBrushEntity fgdClass);
		static bool EndFGDFile();
	};
}
