#include "Node.h"

namespace Nuake
{
	Node::Node()
	{
		Childrens = std::vector<Ref<Node>>();
		Groups = std::vector<std::string>();

		NormalStyle = {
			Layout::PositionType::RELATIVE,
			Layout::LayoutVec4(),
			Layout::LayoutUnit(), Layout::LayoutUnit(), Layout::LayoutUnit(),
			Layout::LayoutUnit(), Layout::LayoutUnit(), Layout::LayoutUnit(),
			Layout::LayoutVec4(),
			Layout::LayoutVec4(),
			Layout::LayoutVec4(),
			Color(),
			Layout::LayoutDirection::LTR,
			Layout::FlexDirection::ROW,
			Layout::FlexWrap::WRAP,
			0.f,
			1.f,
			10.f,
			1.f,
			Layout::JustifyContent(),
			Layout::AlignItems::AUTO,
			Layout::AlignItems::AUTO,
		};
	}

	void Node::ApplyStyle(Ref<StyleGroup> stylegroup, StyleGroupSelector selector)
	{
		Style style = NormalStyle;
		for (auto& s : stylegroup->GetProps())
		{
			switch (s.first)
			{
			case PropType::HEIGHT:
			{
				style.Height.mUnit = (Layout::Unit)(s.second.type);
				float value = s.second.value.Number;
				style.Height.Value = value;
			}
			break;
			case PropType::MIN_HEIGHT:
				style.MinHeight.mUnit = (Layout::Unit)s.second.type;
				style.MinHeight.Value = s.second.value.Number;
				break;
			case PropType::MAX_HEIGHT:
				style.MaxHeight.mUnit = (Layout::Unit)s.second.type;
				style.MaxHeight.Value = s.second.value.Number;
				break;
			case PropType::WIDTH:
				style.Width.mUnit = (Layout::Unit)s.second.type;
				style.Width.Value = s.second.value.Number;
				break;
			case PropType::MIN_WIDTH:
				style.MinWidth.mUnit = (Layout::Unit)s.second.type;
				style.MinWidth.Value = s.second.value.Number;
				break;
			case PropType::MAX_WIDTH:
				style.MaxWidth.mUnit = (Layout::Unit)s.second.type;
				style.MaxWidth.Value = s.second.value.Number;
				break;
			case PropType::MARGIN_RIGHT:
				style.Margin.Right.mUnit = (Layout::Unit)s.second.type;
				style.Margin.Right.Value = s.second.value.Number;
				break;
			case PropType::MARGIN_LEFT:
				style.Margin.Left.mUnit = (Layout::Unit)s.second.type;
				style.Margin.Left.Value = s.second.value.Number;
				break;
			case PropType::MARGIN_TOP:
				style.Margin.Top.mUnit = (Layout::Unit)s.second.type;
				style.Margin.Top.Value = s.second.value.Number;
				break;
			case PropType::MARGIN_BOTTOM:
				style.Margin.Bottom.mUnit = (Layout::Unit)s.second.type;
				style.Margin.Bottom.Value = s.second.value.Number;
				break;
			case PropType::JUSTIFY_CONTENT:
				style.JustifyContent = (Layout::JustifyContent)s.second.value.Enum;
				break;
			case PropType::FLEX_WRAP:
				style.FlexWrap = (Layout::FlexWrap)s.second.value.Enum;
				break;
			case PropType::FLEX_DIRECTION:
				style.FlexDirection = (Layout::FlexDirection)s.second.value.Enum;
				break;
			case PropType::ALIGN_ITEMS:
				style.AlignItems = (Layout::AlignItems)s.second.value.Enum;
				break;
			case PropType::SELF_ALIGN:
				style.AlignSelf = (Layout::AlignItems)s.second.value.Enum;
				break;
			case PropType::ALIGN_CONTENT:
				style.AlignContent = (Layout::AlignContent)s.second.value.Enum;
				break;
			case PropType::POSITION:
				style.PositionType = (Layout::PositionType)s.second.value.Enum;
				break;
			case PropType::LEFT:
				style.Position.Left.mUnit = (Layout::Unit)s.second.type;
				style.Position.Left.Value = s.second.value.Number;
				break;
			case PropType::RIGHT:
				style.Position.Right.mUnit = (Layout::Unit)s.second.type;
				style.Position.Right.Value = s.second.value.Number;
				break;
			case PropType::TOP:
				style.Position.Top.mUnit = (Layout::Unit)s.second.type;
				style.Position.Top.Value = s.second.value.Number;
				break;
			case PropType::BOTTOM:
				style.Position.Bottom.mUnit = (Layout::Unit)s.second.type;
				style.Position.Bottom.Value = s.second.value.Number;
				break;
			}
		}

		if (selector == StyleGroupSelector::Hover)
			HoverStyle = style;
		else
			NormalStyle = style;
	}

	bool Node::IsPositionInside(Vector2 position)
	{
		float width = YGNodeLayoutGetWidth(YogaNode);
		float height = YGNodeLayoutGetHeight(YogaNode);
		float padding = YGNodeLayoutGetPadding(YogaNode, YGEdgeLeft);
		float left = YGNodeLayoutGetLeft(YogaNode); //+ offset.x;
		float top = YGNodeLayoutGetTop(YogaNode);// +offset.y;

		float parentLeft = 0.0f;
		float parentTop = 0.0f;
		auto parent = YGNodeGetParent(YogaNode);
		while (parent)
		{
			parentLeft += YGNodeLayoutGetLeft(parent);
			parentTop += YGNodeLayoutGetTop(parent);
			parent = YGNodeGetParent(parent);
		}

		left += parentLeft;
		top += parentTop;
		return position.x > left && position.x < left + width && position.y > top && position.y < top + height;
	}
}
