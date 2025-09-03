#pragma once
#include "ClassProperty.h"
#include <string>
#include <vector>


class BrushClass {
public:
	std::string name;
	std::vector<ClassProperty> Props;
	std::string Description;
};