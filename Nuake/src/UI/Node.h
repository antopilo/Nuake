#pragma once
#include <vector>
#include "../Core/Core.h"
namespace Layout
{
	enum class LayoutDirection
	{
		LTR, RTL
	};

	enum class FlexWrap
	{
		NO_WRAP, WRAP, WRAP_REVERSED
	};

	enum class FlexDirection
	{
		ROW, COLUMN, ROW_REVERSED, COLUMN_REVERSED
	};

	enum class JustifyContent
	{
		FLEX_START, FLEX_END, CENTER, SPACE_BETWEEN, SPACE_AROUND, SPACE_EVENLY
	};

	enum class AlignItems
	{
		STRETCH, FLEX_START, FLEX_END, CENTER, BASELINE
	};

	enum class AlignContent
	{
		FLEX_START, FLEX_END, STRETCH, CENTER, SPACE_BETWEEN, SPACE_AROUND
	};

	enum class PositionType
	{
		RELATIVE, ABSOLUTE
	};

	enum Unit
	{
		PIXEL, PERCENT, AUTO
	};

	struct LayoutUnit
	{
		float Value = 0.0f;
		Unit Unit = Unit::PIXEL;
	};

	struct LayoutVec4
	{
		LayoutUnit Top = LayoutUnit{ 0, PIXEL };
		LayoutUnit Bottom = LayoutUnit{ 0, PIXEL };
		LayoutUnit Left = LayoutUnit{ 0, PIXEL };
		LayoutUnit Right = LayoutUnit{ 0, PIXEL };
	};
}


// Base UI node.
class Node
{
public:
	std::vector<Ref<Node>> Childrens;
	Layout::PositionType PositionType;
	Layout::LayoutVec4 Position;

	Layout::LayoutUnit With, MaxWidth, MinWidth;
	Layout::LayoutUnit Height, MaxHeight, MinHeight;

	Layout::LayoutVec4 Margin;
	Layout::LayoutVec4 Padding;
	Layout::LayoutVec4 Border;

	Layout::LayoutDirection Direction;

	Layout::FlexDirection FlexDirection;

	Layout::FlexWrap FlexWrap;
	float FlexGrow;
	float FlexShrink;
	float FlexBasis;
	float AspectRatio;

	Layout::AlignItems AlignItems;
	Layout::AlignItems SelfAlign;

	Node();
};

