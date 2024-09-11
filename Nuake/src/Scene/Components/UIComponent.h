#pragma once
#include "src/Core/Object/Object.h"
#include "src/Resource/UUID.h"

namespace Nuake
{
	struct UIComponent
	{
		NUAKECOMPONENT(UIComponent, "UI Component");

		UUID UIResource = UUID(0);
		std::string CSharpUIController;
		std::string UIFilePath;
		bool IsWorldSpace;
		// TODO: Z-Ordering
	};
}