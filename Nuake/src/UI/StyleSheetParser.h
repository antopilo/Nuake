#pragma once
#include <string>
#include "Style.h"

class StyleSheetParser
{

public:
	static PropValue ParsePropType(const std::string& str, PropType type);
};