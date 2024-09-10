#include "Node.h"
#include "../Renderer.h"

#include "NodeState.h"
#include "../StringHelper.h"

#include <nanosvg.h>
#include <nanosvgrast.h>

namespace NuakeUI
{
	NodePtr Node::New(const std::string id, const std::string& value)
	{
		return std::make_shared<Node>(id, value);
	}

	Node::Node(const std::string& id, const std::string& value) : ID(id)
	{
		InitializeNode();

		YGNodeStyleSetFlexDirection(mNode, YGFlexDirection::YGFlexDirectionColumn);
	}

	std::string Node::GetID() const
	{
		return ID;
	}

	uint32_t Node::GetIndex() const
	{
		if (!Parent)
		{
			return -1;
		}

		uint32_t i = 0;
		for (auto& c : Parent->GetChildrens())
		{
			if (c->mNode == mNode)
			{
				return i;
			}

			i++;
		}

		return -1;
	}

	YGNodeRef Node::GetYogaNode() const
	{
		return mNode;
	}

	std::string Node::GetType() const
	{
		return Type;
	}

	void Node::InitializeNode()
	{
		if (mHasBeenInitialized)
			return;

		mNode = YGNodeNew();
		Childrens = std::vector<NodePtr>();
		mHasBeenInitialized = true;
	}

	bool Node::HasBeenInitialized() const
	{
		return mHasBeenInitialized;
	}

	bool Node::HasDataModel() const
	{
		return mDataModel != nullptr;
	}

	DataModelPtr Node::GetDataModel() const
	{
		if (HasDataModel())
		{
			return mDataModel;
		}

		// Ask parent if they have a data model
		if (Parent != nullptr)
		{
			return Parent->GetDataModel();
		}

		return nullptr;
	}

	void Node::SetDataModel(const DataModelPtr& dataModel)
	{
		mDataModel = dataModel;
	}

	float Node::GetScroll() const
	{
		return ScrollDelta;
	}

	DataModelOperationCollection& Node::GetDataModelOperations()
	{
		return mDataModelOperations;
	}

	void Node::AddDataModelOperation(DataModelOperationPtr& operation)
	{
		mDataModelOperations.push_back(operation);
	}

	void Node::Tick(InputManager* inputManager)
	{
		OnTick(inputManager);

		for (auto& c : Childrens)
		{
			c->Tick(inputManager);
		}
	}

	void Node::UpdateInput(InputManager* inputManager)
	{
		inputManager = inputManager;

		float mx = inputManager->GetMouseX();
		float my = inputManager->GetMouseY();
		bool isHover = IsMouseHover(mx, my);

		bool isMouseDown = inputManager->IsMouseInputDown();
		if (State == NodeState::Clicked && !isMouseDown)
		{
			State = NodeState::Hover;
			OnClickReleased(inputManager);
		}

		if (!isMouseDown)
		{
			if (!isHover)
			{
				if (State != NodeState::Idle)
				{
					OnMouseExit(inputManager);
				}

				State = NodeState::Idle;
			}
			else
			{
				if (State != NodeState::Hover)
				{
					OnMouseHover(inputManager);
				}

				State = NodeState::Hover;
			}
		}
		
		// Grab focus
		if (isHover && isMouseDown && State != NodeState::Clicked)
		{
			OnClick(inputManager);
			State = NodeState::Clicked;

			if (CanGrabFocus)
			{
				GrabFocus();
			}
		}

		// Release focus
		if (!isHover && isMouseDown && State != NodeState::Clicked)
		{
			if (HasFocus())
			{
				ReleaseFocus();
			}
		}
		
		// Calculate total absolute height of all the childrens
		float totalHeight = 0.0f;
		for (const auto& c : Childrens)
		{
			float childrenBottom = c->ComputedPosition.y + ScrollDelta + c->ComputedSize.y;
			if (childrenBottom > totalHeight)
			{
				totalHeight = childrenBottom;
			}
		}
		
		// Calculate Max Scroll delta
		float maxScrollDelta = 0.f;
		if (totalHeight > ComputedSize.y)
		{
			maxScrollDelta = -(ComputedSize.y - totalHeight);
		}

		// Scroll the parent and keep the remainder.
		float scroll = inputManager->GetScrollY();

		bool isScrolled = std::abs(scroll) > 0.0f;
		bool canScroll = ComputedStyle.Overflow == OverflowType::Scroll;
		if (isScrolled && canScroll && isHover)
		{
			// Call node event
			OnScroll(inputManager);

			const float SCROLL_FORCE = 40.f;
			const float scrollAmount = scroll * -SCROLL_FORCE;
			const float newDelta = ScrollDelta + scrollAmount;

			// This is what remains of the scrolling if we hit the limits to resolve to parent.
			float remainder = 0.0f;
			if (scrollAmount > .0f)
			{
				if (ScrollDelta < maxScrollDelta && newDelta >= maxScrollDelta)
				{
					remainder = newDelta + maxScrollDelta;
					inputManager->ScrollY += scroll;
					ScrollDelta = maxScrollDelta;
				}
				else if (newDelta <= maxScrollDelta)
				{
					ScrollDelta = newDelta;
					inputManager->ScrollY += scroll;
				}
			}
			else if(scrollAmount < .0f)
			{
				if(ScrollDelta > 0.f && newDelta <= 0.f)
				{
					remainder = newDelta + maxScrollDelta;
					inputManager->ScrollY += scroll;
					ScrollDelta = 0.f;
				}
				else if (newDelta >= 0.f)
				{
					ScrollDelta = newDelta;
					inputManager->ScrollY += scroll;
				}
			}
		}

		if (ScrollDelta > maxScrollDelta)
		{
			ScrollDelta = maxScrollDelta;
		}

		for (auto& c : Childrens)
		{
			c->UpdateInput(inputManager);
		}
	}

	void Node::Draw(int z)
	{
		z++;
		ComputedZIndex = ComputedStyle.ZIndex + z;
		if (ComputedStyle.Visibility == VisibilityType::Hidden)
		{
			return;
		}

		for (const NodePtr& c : Childrens)
		{
			bool failedOperation = false;
			for (const auto& modelOp : c->GetDataModelOperations())
			{
				if (modelOp->Type == OperationType::If)
				{
					const DataModelPtr& model = c->GetDataModel();
					if (model && !modelOp->Compare(model))
					{
						failedOperation = true;
					}
				}
			}
			
			bool isHidden = c->ComputedStyle.Visibility == VisibilityType::Hidden;
			if (isHidden || failedOperation)
			{
				continue;
			}

			Renderer::Get().DrawNode(c, ComputedZIndex);
			c->Draw(ComputedZIndex);
		}
	}

	void Node::Calculate()
	{
		for (auto& c : Childrens)
		{
			c->Calculate();
		}
	}

	bool Node::IsMouseHover(float x, float y)
	{
		YGNodeRef ygNode = GetYogaNode();

		float parentScroll = 0.f;
		if (Parent)
			parentScroll = Parent->ScrollDelta;

		float width   = YGNodeLayoutGetWidth(ygNode);
		float height  = YGNodeLayoutGetHeight(ygNode);
		float padding = YGNodeLayoutGetPadding(ygNode, YGEdgeLeft);
		float left    = YGNodeLayoutGetLeft(ygNode);
		float top = YGNodeLayoutGetTop(ygNode) - parentScroll;

		float parentLeft = 0.0f;
		float parentTop = 0.0f;

		auto parent = Parent;
		if (parent)
		{
			parentLeft = parent->ComputedPosition.x;
			parentTop = parent->ComputedPosition.y;
		}

		left += parentLeft;
		top += parentTop;
		bool isHover = x > left && x < left + width && y > top && y < top + height;
		return isHover;
	}

	void Node::InsertChild(NodePtr child)
	{
		if (!mHasBeenInitialized)
			InitializeNode();

		child->Parent = this;
		Childrens.push_back(child);
		uint32_t index = (uint32_t)Childrens.size() - 1;
		YGNodeInsertChild(this->mNode, child->GetYogaNode(), index);
	}

	Node* Node::mFocused = nullptr;
	void Node::GrabFocus()
	{
		mFocused = this;
	}

	bool Node::HasFocus() const
	{
		return mFocused == this;
	}

	void Node::ReleaseFocus()
	{
		mFocused = nullptr;
	}

	void Node::ApplyStyleProperties(std::map<StyleProperties, PropValue> properties)
	{
		for (auto& p : properties)
		{
			StyleProperties prop = p.first;
			PropValue value = p.second;

			switch (prop)
			{
				LengthProp(Width)
				LengthProp(Height)
				LengthProp(MinHeight)
				LengthProp(MinWidth)
				LengthProp(MaxHeight)
				LengthProp(MaxWidth)
				LengthProp(MarginLeft)
				LengthProp(MarginTop)
				LengthProp(MarginRight)
				LengthProp(MarginBottom)
				LengthProp(PaddingLeft)
				LengthProp(PaddingTop)
				LengthProp(PaddingRight)
				LengthProp(PaddingBottom)
				case StyleProperties::Top:
					ComputedStyle.Top = value.value.Number;
					break;
				case StyleProperties::Bottom:
					ComputedStyle.Bottom = value.value.Number;
					break;
				case StyleProperties::Left:
					ComputedStyle.Left = value.value.Number;
					break;
				case StyleProperties::Right:
					ComputedStyle.Right = value.value.Number;
				break;
				EnumProp(Position)
				EnumProp(AlignItems)
				EnumPropEx(SelfAlign, AlignItemsType)
				case StyleProperties::AspectRatio:
					ComputedStyle.AspectRatio = value.value.Number;
					break;
				EnumProp(FlexDirection)
				EnumProp(FlexWrap)
				case StyleProperties::FlexBasis:
					ComputedStyle.FlexBasis = value.value.Number;
					break;
				case StyleProperties::FlexGrow:
					ComputedStyle.FlexGrow = value.value.Number;
					break;		
				case StyleProperties::FlexShrink:
						ComputedStyle.FlexShrink = value.value.Number;
						break;
				EnumProp(JustifyContent)
				EnumProp(AlignContent)
				EnumProp(LayoutDirection)
				case StyleProperties::BorderSize:
					ComputedStyle.BorderSize = std::clamp(value.value.Number, 0.f, ComputedSize.x / 2.0f);
					break;
				case StyleProperties::BorderRadius:
					ComputedStyle.BorderRadius = value.value.Number;
					break;
				case StyleProperties::BorderColor:
					ComputedStyle.BorderColor = value.value.Color / 255.f;
					break;
				case StyleProperties::BackgroundColor:
					this->ComputedStyle.BackgroundColor = value.value.Color / 255.f;
					break;
				case StyleProperties::Color:
					ComputedStyle.FontColor = value.value.Color / 255.f;
					break;
				case StyleProperties::TextAlign:
					ComputedStyle.TextAlign = (TextAlignType)(value.value.Enum);
					break;
				case StyleProperties::Overflow:
					ComputedStyle.Overflow = (OverflowType)value.value.Enum;
					break;
				case StyleProperties::FontSize:
					ComputedStyle.FontSize = value.value.Number;
					break;
				case StyleProperties::Visibility:
					ComputedStyle.Visibility = (VisibilityType)value.value.Enum;
					break;
				case StyleProperties::ZIndex:
					ComputedStyle.ZIndex = value.value.Number;
					break;
				case StyleProperties::BackgroundImage:
					{
						if (ComputedStyle.BackgroundImage == nullptr)
						{
							if (StringHelper::EndsWith(value.string, ".svg"))
							{
								//NSVGimage* image = NULL;
								//NSVGrasterizer* rast = NULL;
								//unsigned char* img = NULL;
								//
								//int w, h;
								//image = nsvgParseFromFile(value.string.c_str(), "px", 96.0f);
								//if (image == NULL) {
								//	printf("Could not open SVG image.\n");
								//
								//}
								//w = (int)image->width;
								//h = (int)image->height;
								//
								//rast = nsvgCreateRasterizer();
								//if (rast == NULL) {
								//	printf("Could not init rasterizer.\n");
								//}
								//
								//img = (unsigned char*)malloc(w * h * 4);
								//if (img == NULL) {
								//	printf("Could not alloc image buffer.\n");
								//}
								//
								//nsvgRasterize(rast, image, 0, 0, 1, img, w, h, w * 4);
								//
								//auto texture = std::make_shared<Texture>(img, w * h * 4);
								//ComputedStyle.BackgroundImage = texture;
								//
								//nsvgDeleteRasterizer(rast);
								//nsvgDelete(image);
							}
							else
							{
								auto texture = std::make_shared<Texture>(value.string);
								ComputedStyle.BackgroundImage = texture;
							}
						}
					}
					break;
			}
		}


		SetLength(Width)
		SetLength(Height)
		
		if (ComputedStyle.Position == PositionType::Absolute)
		{
			if (ComputedStyle.Top != -1)
			{
				YGNodeStyleSetPosition(mNode, YGEdgeTop, ComputedStyle.Top);
			}
			if (ComputedStyle.Bottom != -1)
			{
				YGNodeStyleSetPosition(mNode, YGEdgeBottom, ComputedStyle.Bottom);
			}
			if (ComputedStyle.Left != -1)
			{
				YGNodeStyleSetPosition(mNode, YGEdgeLeft, ComputedStyle.Left);
			}
			if (ComputedStyle.Right != -1)
			{
				YGNodeStyleSetPosition(mNode, YGEdgeRight, ComputedStyle.Right);
			}
		}

		SetLengthNoAuto(MaxWidth)
		SetLengthNoAuto(MaxHeight)
		SetLengthNoAuto(MinWidth)
		SetLengthNoAuto(MinHeight)
		SetLengthNoAuto(MaxWidth)
		SetLengthNoAuto(MaxHeight)
		SetLengthBorder(Margin, Left);
		SetLengthBorder(Margin, Top);
		SetLengthBorder(Margin, Right);
		SetLengthBorder(Margin, Bottom);
		SetLengthBorderNoAuto(Padding, Left);
		SetLengthBorderNoAuto(Padding, Top);
		SetLengthBorderNoAuto(Padding, Right);
		SetLengthBorderNoAuto(Padding, Bottom);

		if (ComputedStyle.Position == PositionType::Relative)
			YGNodeStyleSetPositionType(mNode, YGPositionTypeRelative);
		else if (ComputedStyle.Position == PositionType::Absolute)
			YGNodeStyleSetPositionType(mNode, YGPositionTypeAbsolute);
		if (ComputedStyle.FlexDirection == FlexDirectionType::Column)
			YGNodeStyleSetFlexDirection(mNode, YGFlexDirectionColumn);
		else if (ComputedStyle.FlexDirection == FlexDirectionType::ColumnReversed)
			YGNodeStyleSetFlexDirection(mNode, YGFlexDirectionColumnReverse);
		else if (ComputedStyle.FlexDirection == FlexDirectionType::Row)
			YGNodeStyleSetFlexDirection(mNode, YGFlexDirectionRow);
		else if (ComputedStyle.FlexDirection == FlexDirectionType::RowReversed)
			YGNodeStyleSetFlexDirection(mNode, YGFlexDirectionRowReverse);

		if (ComputedStyle.JustifyContent == JustifyContentType::FlexStart)
			YGNodeStyleSetJustifyContent(mNode, YGJustifyFlexStart);
		else if (ComputedStyle.JustifyContent == JustifyContentType::Center)
			YGNodeStyleSetJustifyContent(mNode, YGJustifyCenter);
		else if (ComputedStyle.JustifyContent == JustifyContentType::FlexEnd)
			YGNodeStyleSetJustifyContent(mNode, YGJustifyFlexEnd);
		else if (ComputedStyle.JustifyContent == JustifyContentType::SpaceAround)
			YGNodeStyleSetJustifyContent(mNode, YGJustifySpaceAround);
		else if (ComputedStyle.JustifyContent == JustifyContentType::SpaceBetween)
			YGNodeStyleSetJustifyContent(mNode, YGJustifySpaceBetween);
		else if (ComputedStyle.JustifyContent == JustifyContentType::SpaceEvenly)
			YGNodeStyleSetJustifyContent(mNode, YGJustifySpaceEvenly);

		if (ComputedStyle.AlignItems == AlignItemsType::FlexStart)
			YGNodeStyleSetAlignItems(mNode, YGAlignFlexStart);
		else if (ComputedStyle.AlignItems == AlignItemsType::Center)
			YGNodeStyleSetAlignItems(mNode, YGAlignCenter);
		else if (ComputedStyle.AlignItems == AlignItemsType::FlexEnd)
			YGNodeStyleSetAlignItems(mNode, YGAlignFlexEnd);
		else if (ComputedStyle.AlignItems == AlignItemsType::SpaceAround)
			YGNodeStyleSetAlignItems(mNode, YGAlignSpaceAround);
		else if (ComputedStyle.AlignItems == AlignItemsType::SpaceBetween)
			YGNodeStyleSetAlignItems(mNode, YGAlignSpaceBetween);
		else if (ComputedStyle.AlignItems == AlignItemsType::Stretch)
			YGNodeStyleSetAlignItems(mNode, YGAlignStretch);
		else if (ComputedStyle.AlignItems == AlignItemsType::Baseline)
			YGNodeStyleSetAlignItems(mNode, YGAlignBaseline);

		if (ComputedStyle.FlexDirection == FlexDirectionType::Row)
			YGNodeStyleSetFlexDirection(mNode, YGFlexDirectionRow);
		if (ComputedStyle.FlexDirection == FlexDirectionType::RowReversed)
			YGNodeStyleSetFlexDirection(mNode, YGFlexDirectionRowReverse);
		if (ComputedStyle.FlexDirection == FlexDirectionType::Column)
			YGNodeStyleSetFlexDirection(mNode, YGFlexDirectionColumn);
		if (ComputedStyle.FlexDirection == FlexDirectionType::ColumnReversed)
			YGNodeStyleSetFlexDirection(mNode, YGFlexDirectionColumnReverse);

		else if (ComputedStyle.AlignItems == AlignItemsType::Center)
			YGNodeStyleSetAlignItems(mNode, YGAlignCenter);
		else if (ComputedStyle.AlignItems == AlignItemsType::FlexEnd)
			YGNodeStyleSetAlignItems(mNode, YGAlignFlexEnd);
		else if (ComputedStyle.AlignItems == AlignItemsType::SpaceAround)
			YGNodeStyleSetAlignItems(mNode, YGAlignSpaceAround);
		else if (ComputedStyle.AlignItems == AlignItemsType::SpaceBetween)
			YGNodeStyleSetAlignItems(mNode, YGAlignSpaceBetween);
		else if (ComputedStyle.AlignItems == AlignItemsType::Stretch)
			YGNodeStyleSetAlignItems(mNode, YGAlignStretch);
		else if (ComputedStyle.AlignItems == AlignItemsType::Baseline)
			YGNodeStyleSetAlignItems(mNode, YGAlignBaseline);
	}

	std::vector<NodePtr> Node::GetChildrens() const
	{
		return Childrens;
	}
}