#pragma once
#include <string>

enum class ClassPropertyType {
	Boolean,
	String,
	Float,
	Integer,
	AABB,
	Choices,
	Color,
};

enum class FGDClassType {
	Point,
	Brush
};

struct ClassProperty {
	std::string name;
	ClassPropertyType type;
	std::string description;
	std::string value;
};