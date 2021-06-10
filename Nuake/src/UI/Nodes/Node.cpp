#include "Node.h"

Node::Node()
{
	Childrens = std::vector<Ref<Node>>();
	Groups = std::vector<std::string>();
	PositionType = Layout::PositionType::RELATIVE;
	Position = Layout::LayoutVec4();
	Height = MaxHeight = MinHeight = Layout::LayoutUnit();
	Margin = Padding = Border = Layout::LayoutVec4();
	Direction = Layout::LayoutDirection::LTR;
	FlexDirection = Layout::FlexDirection::ROW;
	FlexWrap = Layout::FlexWrap::WRAP;
	FlexGrow = 0.f;
	FlexShrink = 1.f;
	FlexBasis = 10.f;
	AspectRatio = 1.f;
	AlignItems = Layout::AlignItems::AUTO;
	AlignSelf = Layout::AlignItems::AUTO;
}

void Node::ApplyStyle(Ref<StyleGroup> stylegroup)
{
	for (auto& s : stylegroup->GetProps())
	{
		switch (s.first)
		{
		case PropType::HEIGHT:
		{
			Height.Unit = (Layout::Unit)(s.second.type);
			float value = s.second.value.Number;
			Height.Value = value;
		}
		break;
		case PropType::MIN_HEIGHT:
			MinHeight.Unit = (Layout::Unit)s.second.type;
			MinHeight.Value = s.second.value.Number;
			break;
		case PropType::MAX_HEIGHT:
			MaxHeight.Unit = (Layout::Unit)s.second.type;
			MaxHeight.Value = s.second.value.Number;
			break;
		case PropType::WIDTH:
			Width.Unit = (Layout::Unit)s.second.type;
			Width.Value = s.second.value.Number;
			break;
		case PropType::MIN_WIDTH:
			MinWidth.Unit = (Layout::Unit)s.second.type;
			MinWidth.Value = s.second.value.Number;
			break;
		case PropType::MAX_WIDTH:
			MaxWidth.Unit = (Layout::Unit)s.second.type;
			MaxWidth.Value = s.second.value.Number;
			break;
		case PropType::MARGIN_RIGHT:
			Margin.Right.Unit = (Layout::Unit)s.second.type;
			Margin.Right.Value = s.second.value.Number;
			break;
		case PropType::MARGIN_LEFT:
			Margin.Left.Unit = (Layout::Unit)s.second.type;
			Margin.Left.Value = s.second.value.Number;
			break;
		case PropType::MARGIN_TOP:
			Margin.Top.Unit = (Layout::Unit)s.second.type;
			Margin.Top.Value = s.second.value.Number;
			break;
		case PropType::MARGIN_BOTTOM:
			Margin.Bottom.Unit = (Layout::Unit)s.second.type;
			Margin.Bottom.Value = s.second.value.Number;
			break;
		case PropType::JUSTIFY_CONTENT:
			JustifyContent = (Layout::JustifyContent)s.second.value.Enum;
			break;
		case PropType::FLEX_WRAP:
			FlexWrap = (Layout::FlexWrap)s.second.value.Enum;
			break;
		case PropType::FLEX_DIRECTION:
			FlexDirection = (Layout::FlexDirection)s.second.value.Enum;
			break;
		case PropType::ALIGN_ITEMS:
			AlignItems = (Layout::AlignItems)s.second.value.Enum;
			break;
		case PropType::SELF_ALIGN:
			AlignSelf = (Layout::AlignItems)s.second.value.Enum;
			break;
		case PropType::ALIGN_CONTENT:
			AlignContent = (Layout::AlignContent)s.second.value.Enum;
			break;
		case PropType::POSITION:
			PositionType = (Layout::PositionType)s.second.value.Enum;
			break;
		case PropType::LEFT:
			Position.Left.Unit = (Layout::Unit)s.second.type;
			Position.Left.Value = s.second.value.Number;
			break;
		case PropType::RIGHT:
			Position.Right.Unit = (Layout::Unit)s.second.type;
			Position.Right.Value = s.second.value.Number;
			break;
		case PropType::TOP:
			Position.Top.Unit = (Layout::Unit)s.second.type;
			Position.Top.Value = s.second.value.Number;
			break;
		case PropType::BOTTOM:
			Position.Bottom.Unit = (Layout::Unit)s.second.type;
			Position.Bottom.Value = s.second.value.Number;
			break;
		}
	}
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
	if (parent)
	{
		parentLeft = YGNodeLayoutGetLeft(YGNodeGetParent(YogaNode));
		parentTop = YGNodeLayoutGetTop(YGNodeGetParent(YogaNode));
		float parentPaddingTop = YGNodeLayoutGetPadding(parent, YGEdgeTop);
		float parentPaddingLeft = YGNodeLayoutGetPadding(parent, YGEdgeTop);
		// Overflow hidden.
		float parentwidth = YGNodeLayoutGetWidth(parent);
		if (parentwidth - YGNodeLayoutGetMargin(YogaNode, YGEdgeLeft) < width)
			width = parentwidth - parentPaddingLeft;
		float parentHeight = YGNodeLayoutGetHeight(parent);
		if (parentHeight < height)
			height = parentHeight - parentPaddingTop;
	}

	left += parentLeft;
	top += parentTop;
	return position.x > left && position.x < left + width && position.y > top && position.y < top + height;
}