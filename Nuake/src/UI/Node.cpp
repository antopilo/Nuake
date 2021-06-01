#include "Node.h"

Node::Node()
{
	Childrens = std::vector<Ref<Node>>();

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
	AlignItems = Layout::AlignItems::FLEX_START;
	SelfAlign = Layout::AlignItems::FLEX_START;
}