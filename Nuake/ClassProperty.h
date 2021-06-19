#pragma once
#include <string>
enum class ClassPropertyType {
	AABB,
	Float,
	String,
	DropDown,
	Color,
	Int
};

struct ClassProperty {
	std::string name;
	ClassPropertyType type;
};