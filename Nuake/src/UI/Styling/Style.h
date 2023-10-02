#pragma once
#include "../Core/Maths.h"
#include <map>

namespace Nuake
{
	namespace Layout
	{
		enum class LayoutDirection
		{
			LTR = 2, RTL = 3
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
			STRETCH, FLEX_START, FLEX_END, CENTER, BASELINE, AUTO,
		};

		enum class AlignContent
		{
			FLEX_START, FLEX_END, STRETCH, CENTER, SPACE_BETWEEN, SPACE_AROUND
		};

		enum class PositionType
		{
			STATIC, RELATIVE, ABSOLUTE
		};

		enum Unit
		{
			PIXEL, PERCENT, AUTO
		};

		struct LayoutUnit
		{
			float Value = 0.0f;
			Unit mUnit = Unit::PIXEL;
		};

		struct LayoutVec4
		{
			LayoutUnit Top = LayoutUnit{ 0, PIXEL };
			LayoutUnit Bottom = LayoutUnit{ 0, PIXEL };
			LayoutUnit Left = LayoutUnit{ 0, PIXEL };
			LayoutUnit Right = LayoutUnit{ 0, PIXEL };
		};
	}

	enum class PropType
	{
		NONE, HEIGHT, MAX_HEIGHT, MIN_HEIGHT, WIDTH, MAX_WIDTH, MIN_WIDTH,
		LAYOUT_DIRECTION, JUSTIFY_CONTENT,
		FLEX_BASIS, FLEX_GROW, FLEX_SHRINK, FLEX_WRAP, FLEX_DIRECTION,
		ASPECT_RATIO,
		ALIGN_ITEMS, SELF_ALIGN, ALIGN_CONTENT,
		POSITION,
		LEFT, RIGHT, TOP, BOTTOM,
		MARGIN, MARGIN_LEFT, MARGIN_RIGHT, MARGIN_TOP, MARGIN_BOTTOM,
		PADDING, PADDING_LEFT, PADDING_RIGHT, PADDING_TOP, PADDING_BOTTOM,
		BORDER, BORDER_LEFT, BORDER_RIGHT, BORDER_TOP, BORDER_BOTTOM,
		BORDER_RADIUS, BORDER_RADIUS_LEFT, BORDER_RADIUS_RIGHT, BORDER_RADIUS_TOP, BORDER_RADIUS_BOTTOM,
		BACKGROUND_COLOR, COLOR, FONT_SIZE
	};

	enum class PropValueType
	{
		PIXEL, PERCENT, AUTO, COLOR, ENUM
	};
	union Value
	{
		float Number;
		Color Color_;
		int Enum;
	};
	struct PropValue
	{
		PropValueType type;
		Value value;
	};

	enum class StyleGroupSelector
	{
		Normal,
		Hover,
		Active
	};

	class StyleGroup
	{
	public:
		std::map<PropType, PropValue> Props;
		StyleGroupSelector Selector;

		StyleGroup()
		{
			this->Props = std::map<PropType, PropValue>();
		}

		void SetProp(PropType type, PropValue prop)
		{
			Props[type] = prop;
		}

		PropValue GetProp(PropType type)
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


	struct Style
	{
		Layout::PositionType PositionType;
		Layout::LayoutVec4 Position;
		Layout::LayoutUnit Width;
		Layout::LayoutUnit MaxWidth;
		Layout::LayoutUnit MinWidth;
		Layout::LayoutUnit Height; 
		Layout::LayoutUnit MaxHeight;
		Layout::LayoutUnit MinHeight;
		Layout::LayoutVec4 Margin;
		Layout::LayoutVec4 Padding;
		Layout::LayoutVec4 Border;
		Color BackgroundColor;
		Layout::LayoutDirection Direction;
		Layout::FlexDirection FlexDirection;
		Layout::FlexWrap FlexWrap;
		float FlexGrow;
		float FlexShrink;
		float FlexBasis;
		float AspectRatio;
		Layout::JustifyContent JustifyContent;
		Layout::AlignItems AlignItems;
		Layout::AlignItems AlignSelf;
		Layout::AlignContent AlignContent;
	};
}
