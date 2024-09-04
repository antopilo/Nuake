#pragma once
#include "src/Core/Core.h"

namespace Nuake
{
	class FileDialog
	{
	public:
		static std::string OpenFile(const std::string_view& filter);
		static std::string SaveFile(const std::string_view& filter);

		static std::string OpenFolder();
	};
}
