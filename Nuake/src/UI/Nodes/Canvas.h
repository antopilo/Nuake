#pragma once
#include "Node.h"
#include "../Core/Core.h"
#include "../Scripting/WrenScript.h"
#include <map>
// Base container for UI.
class Canvas : public Node
{
private:
	// Script here

public:
	Ref<WrenScript> Script;
	Canvas();
};