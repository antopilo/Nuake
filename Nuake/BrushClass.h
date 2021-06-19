#pragma once
#include "BaseClass.h"
#include "ClassProperty.h"
#include <string>
#include <vector>


class BrushClass : public BaseClass {
	std::string name;
	std::vector<ClassProperty> Props;
	std::string Description;
};