#pragma once
#include "Node.h"
#include "../Core/Core.h"
#include "../Scripting/WrenScript.h"
#include <map>
#include "../Styling/Stylesheet.h"

namespace Nuake {
	class Canvas : public Node
	{
	private:

	public:
		Ref<WrenScript> Script;
		Ref<UI::StyleSheet> StyleSheet;

		Canvas();
	};
}
