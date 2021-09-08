#pragma once
#include "Node.h"
#include "src/Core/Core.h"
#include "src/Scripting/WrenScript.h"
#include "src/UI/Styling/Stylesheet.h"

#include <vector>
#include <string>

namespace Nuake {
	class Canvas : public Node
	{
	private:

	public:
		std::vector<std::pair<std::string, std::string>> ScriptsToLoad;

		Ref<WrenScript> Script;
		Ref<UI::StyleSheet> StyleSheet;

		Canvas();
	};
}
