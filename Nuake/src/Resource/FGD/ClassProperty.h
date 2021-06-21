#pragma once
#include <string>
enum class ClassPropertyType {
	String,
	Integer,
	Float,
	Boolean,
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
};