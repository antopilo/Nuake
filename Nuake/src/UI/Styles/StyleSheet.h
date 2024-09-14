#pragma once
#include "../Nodes/NodeState.h"

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <src/Core/Maths.h>

using namespace Nuake;

namespace NuakeUI
{
	enum class StyleProperties {
		None, Width, Height, MinWidth, MinHeight, MaxWidth, MaxHeight,
		Top, Bottom, Left, Right,
		PaddingLeft, PaddingTop, PaddingRight, PaddingBottom,
		MarginLeft, MarginTop, MarginRight, MarginBottom,
		Position, AlignItems, SelfAlign,
		AspectRatio, FlexDirection, FlexWrap, FlexBasis, FlexGrow, FlexShrink,
		JustifyContent, AlignContent, LayoutDirection,
		BorderSize, BorderRadius, BorderColor,
		BackgroundColor, TextAlign, Color, Overflow, FontSize, Visibility, ZIndex, 
		BackgroundImage, Font
	};
	enum class PositionType { Relative, Absolute };
	enum class AlignItemsType { Auto, FlexStart, Center, FlexEnd, Stretch, Baseline, SpaceBetween, SpaceAround };
	enum class FlexWrapType { NoWrap, Wrap, WrapReversed };
	enum class JustifyContentType { FlexStart, FlexEnd, Center, SpaceBetween, SpaceAround, SpaceEvenly };
	enum class FlexDirectionType { Row = 0, Column, RowReversed, ColumnReversed };
	enum class AlignContentType { FlexStart, FlexEnd, Stretch, Center, SpaceBetween, SpaceAround };
	enum class LayoutDirectionType { LTR = 2, RTL = 3 };

	enum class TextAlignType
	{
		Left, Center, Right
	};

	enum class OverflowType { Show, Hidden, Scroll };
	enum class VisibilityType {Show, Hidden};

	// Style Values
	enum class PropValueType { Pixel , Percent, Auto, Color, Enum, String };

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
		std::string string;

		PropValue()
		{

		}

		PropValue(PropValueType t, float f)
		{
			type = t;
			value.Number = f;
		}

		PropValue(int i)
		{
			type = PropValueType::Enum;
			value.Enum = i;
		}

		PropValue(Color c)
		{
			type = PropValueType::Color;
			value.Color = c;
		}


	};

	// Selector
	enum class StyleSelectorType { Id, Class, Tag, Pseudo };
	enum class Relation { None, Descendant, Child, SubSelection };
	struct StyleSelector
	{
		StyleSelectorType Type;
		std::string Value;
		Relation SelectorRelation;
	};

	class StyleRule
	{
	public:
		std::vector<StyleSelector> Selector;

		std::map<StyleProperties, PropValue> Properties;

		StyleRule(std::vector<StyleSelector> selector) : Selector(selector)
		{
			Properties = std::map<StyleProperties, PropValue>();
		}

		void SetProp(StyleProperties prop, PropValue value)
		{
			Properties[prop] = value;
		}

		PropValue GetProp(StyleProperties prop)
		{
			if (HasProp(prop))
				return Properties[prop];

			return PropValue();
		}

		bool HasProp(StyleProperties prop)
		{
			return Properties.find(prop) != Properties.end();
		}
	};

	class StyleSheet;
	typedef std::shared_ptr<StyleSheet> StyleSheetPtr;
	class StyleSheet
	{
	public:
		std::vector<StyleRule> Rules;

		static StyleSheetPtr New()
		{
			return std::make_shared<StyleSheet>();
		}

		StyleSheet() = default;
		~StyleSheet() = default;
	};
}