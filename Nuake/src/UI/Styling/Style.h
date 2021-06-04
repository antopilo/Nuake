#pragma once
#include "../Core/Maths.h"
#include <map>
enum class PropType
{
	HEIGHT, MAX_HEIGHT, MIN_HEIGHT, WIDTH, MAX_WIDTH, MIN_WIDTH,
	LAYOUT_DIRECTION, JUSTIFY_CONTENT,
	FLEX_BASIS, FLEX_GROW, FLEX_SHRINK,FLEX_WRAP, FLEX_DIRECTION, 
	ASPECT_RATIO,
	ALIGN_ITEMS, SELF_ALIGN, ALIGN_CONTENT, 
	POSITION,
	LEFT, RIGHT, TOP, BOTTOM,
	MARGIN, MARGIN_LEFT, MARGIN_RIGHT, MARGIN_TOP, MARGIN_BOTTOM,
	PADDING, PADDING_LEFT, PADDING_RIGHT, PADDING_TOP, PADDING_BOTTOM,
	BORDER, BORDER_LEFT, BORDER_RIGHT, BORDER_TOP, BORDER_BOTTOM,
	BORDER_RADIUS, BORDER_RADIUS_LEFT, BORDER_RADIUS_RIGHT, BORDER_RADIUS_TOP, BORDER_RADIUS_BOTTOM,
	BACKGROUND_COLOR, COLOR, 
};

enum class PropValueType
{
	PIXEL, PERCENT, AUTO, COLOR, ENUM
};
union Value
{
	float Number;
	Color Color;
	int Enum;
};
struct PropValue
{
	PropValueType type;
	Value value;
};

class StyleGroup
{
public:
	std::map<PropType, PropValue> Props;

	StyleGroup()
	{
		this->Props = std::map<PropType, PropValue>();
	}

	void SetProp(PropType type, PropValue prop)
	{
		Props[type] = prop;
	}

	PropValue& GetProp(PropType type)
	{
		if (Props.find(type) != Props.end())
			return Props[type];

		return PropValue();
	}

	bool HasProp(PropType type)
	{
		return Props.find(type) != Props.end();
	}

	std::map<PropType, PropValue> GetProps()
	{
		return Props;
	}

	StyleGroup operator+(StyleGroup other) 
	{
		for (auto p : other.Props)
		{
			this->Props[p.first] = p.second;
		}

		return *this; 
	}
};


class Style
{

};