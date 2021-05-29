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
	FlexShrink = 0.f;
	FlexBasis = 0.f;
	AspectRatio = 0.f;
	AlignItems = Layout::AlignItems::FLEX_START;
	SelfAlign = Layout::AlignItems::FLEX_START;
}