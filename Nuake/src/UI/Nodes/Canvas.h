#pragma once
#include "Node.h"
#include "../Core/Core.h"
#include "../Scripting/WrenScript.h"
#include <map>
#include "../Styling/Stylesheet.h"

// Base container for UI.
class Canvas : public Node
{
private:
	// Script here

public:
	Ref<WrenScript> Script;
	Ref<UI::StyleSheet> StyleSheet;

	Canvas();
};