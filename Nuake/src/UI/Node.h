#pragma once
#include <vector>
#include "../Core/Core.h"
#include "yoga/Yoga.h"
#include "../Rendering/Renderer2D.h"
#include "../Core/Logger.h"
#include "Style.h"

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
		STRETCH, FLEX_START, FLEX_END, CENTER, BASELINE
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
	Ref<Node*> Parent;
	YGNodeRef YogaNode;
	std::vector<Ref<Node>> Childrens = std::vector<Ref<Node>>();

	void ApplyStyle(Ref<StyleGroup> stylegroup);
	Ref<Style> style;
	std::vector<std::string> Groups;

	Layout::PositionType PositionType;
	Layout::LayoutVec4 Position;

	Layout::LayoutUnit Width, MaxWidth, MinWidth;
	Layout::LayoutUnit Height, MaxHeight, MinHeight;

	Layout::LayoutVec4 Margin;
	Layout::LayoutVec4 Padding;
	Layout::LayoutVec4 Border;

	Color BackgroundColor;

	Layout::LayoutDirection Direction;

	Layout::FlexDirection FlexDirection = Layout::FlexDirection::ROW;

	Layout::FlexWrap FlexWrap;
	float FlexGrow;
	float FlexShrink;
	float FlexBasis;
	float AspectRatio;

	Layout::AlignItems AlignItems;
	Layout::AlignItems SelfAlign;

	Node();

	void AddGroup(const std::string& group)
	{
		Groups.push_back(group);
	}

	bool IsInGroup(const std::string& group)
	{
		for (auto& g : Groups)
			if (g == group) return true;
		return false;
	}

	std::vector<std::string> GetGroups() const { return Groups; }

	void SetMargin()
	{
		switch (Margin.Left.Unit)
		{
		case Layout::Unit::PIXEL:
			YGNodeStyleSetMargin(YogaNode, YGEdgeLeft, Margin.Left.Value);
			break;
		case Layout::Unit::PERCENT:
			YGNodeStyleSetMarginPercent(YogaNode, YGEdgeLeft, Margin.Left.Value);
			break;
		case Layout::Unit::AUTO:
			YGNodeStyleSetMarginAuto(YogaNode, YGEdgeLeft);
			break;
		}

		switch (Margin.Right.Unit)
		{
		case Layout::Unit::PIXEL:
			YGNodeStyleSetMargin(YogaNode, YGEdgeRight, Margin.Right.Value);
			break;
		case Layout::Unit::PERCENT:
			YGNodeStyleSetMarginPercent(YogaNode, YGEdgeRight, Margin.Right.Value);
			break;
		case Layout::Unit::AUTO:
			YGNodeStyleSetMarginAuto(YogaNode, YGEdgeRight);
			break;
		}

		switch (Margin.Top.Unit)
		{
		case Layout::Unit::PIXEL:
			YGNodeStyleSetMargin(YogaNode, YGEdgeTop, Margin.Top.Value);
			break;
		case Layout::Unit::PERCENT:
			YGNodeStyleSetMarginPercent(YogaNode, YGEdgeTop, Margin.Top.Value);
			break;
		case Layout::Unit::AUTO:
			YGNodeStyleSetMarginAuto(YogaNode, YGEdgeTop);
			break;
		}

		switch (Margin.Bottom.Unit)
		{
		case Layout::Unit::PIXEL:
			YGNodeStyleSetMargin(YogaNode, YGEdgeBottom, Margin.Bottom.Value);
			break;
		case Layout::Unit::PERCENT:
			YGNodeStyleSetMarginPercent(YogaNode, YGEdgeBottom, Margin.Bottom.Value);
			break;
		case Layout::Unit::AUTO:
			YGNodeStyleSetMarginAuto(YogaNode, YGEdgeTop);
			break;
		}

	}

	void SetPadding()
	{
		switch (Padding.Left.Unit)
		{
		case Layout::Unit::PIXEL:
			YGNodeStyleSetPadding(YogaNode, YGEdgeLeft, Padding.Left.Value);
			break;
		case Layout::Unit::PERCENT:
			YGNodeStyleSetPaddingPercent(YogaNode, YGEdgeLeft, Padding.Left.Value);
			break;
		}

		switch (Padding.Right.Unit)
		{
		case Layout::Unit::PIXEL:
			YGNodeStyleSetPadding(YogaNode, YGEdgeRight, Padding.Right.Value);
			break;
		case Layout::Unit::PERCENT:
			YGNodeStyleSetPaddingPercent(YogaNode, YGEdgeRight, Padding.Right.Value);
			break;
		}

		switch (Padding.Top.Unit)
		{
		case Layout::Unit::PIXEL:
			YGNodeStyleSetPadding(YogaNode, YGEdgeTop, Padding.Top.Value);
			break;
		case Layout::Unit::PERCENT:
			YGNodeStyleSetPaddingPercent(YogaNode, YGEdgeTop, Padding.Top.Value);
			break;
		}

		switch (Padding.Bottom.Unit)
		{
		case Layout::Unit::PIXEL:
			YGNodeStyleSetPadding(YogaNode, YGEdgeBottom, Padding.Bottom.Value);
			break;
		case Layout::Unit::PERCENT:
			YGNodeStyleSetPaddingPercent(YogaNode, YGEdgeBottom, Padding.Bottom.Value);
			break;
		}
	}

	void SetBorder()
	{
		switch (Border.Left.Unit)
		{
		case Layout::Unit::PIXEL:
			YGNodeStyleSetBorder(YogaNode, YGEdgeLeft, Border.Left.Value);
			break;
		}

		switch (Border.Right.Unit)
		{
		case Layout::Unit::PIXEL:
			YGNodeStyleSetBorder(YogaNode, YGEdgeRight, Border.Right.Value);
			break;
		}

		switch (Border.Top.Unit)
		{
		case Layout::Unit::PIXEL:
			YGNodeStyleSetBorder(YogaNode, YGEdgeTop, Border.Top.Value);
			break;
		}

		switch (Border.Bottom.Unit)
		{
		case Layout::Unit::PIXEL:
			YGNodeStyleSetBorder(YogaNode, YGEdgeBottom, Border.Bottom.Value);
			break;
		}
	}

	void SetYogaLayout()
	{
		switch (Width.Unit)
		{
		case Layout::Unit::PIXEL:
			YGNodeStyleSetWidth(YogaNode, Width.Value);
			break;
		case Layout::Unit::PERCENT:
			YGNodeStyleSetWidthPercent(YogaNode, Width.Value);
			break;
		case Layout::Unit::AUTO:
			YGNodeStyleSetWidthAuto(YogaNode);
			break;
		}

		switch (Height.Unit)
		{
		case Layout::Unit::PIXEL:
			YGNodeStyleSetHeight(YogaNode, Height.Value);
			break;
		case Layout::Unit::PERCENT:
			YGNodeStyleSetHeightPercent(YogaNode, Height.Value);
			break;
		case Layout::Unit::AUTO:
			YGNodeStyleSetHeightAuto(YogaNode);
			break;
		}

		SetMargin();
		SetPadding();
		SetBorder();

		YGNodeStyleSetFlexWrap(YogaNode, YGWrapWrap);
		YGNodeStyleSetFlexDirection(YogaNode, YGFlexDirectionRow);
		YGNodeStyleSetJustifyContent(YogaNode, YGJustifyFlexStart);
	}

	void Draw(float z, Vector2 offset)
	{
		Color color = this->BackgroundColor;
		Renderer2D::UIShader->SetUniform4f("background_color",
			color.r / 255.f,
			color.g / 255.f,
			color.b / 255.f,
			color.a / 255.f);

		// Get transform
		Matrix4 transform = Matrix4(1.0f);
		
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
			if (parentwidth - YGNodeLayoutGetMargin(YogaNode, YGEdgeLeft) < width )
				width = parentwidth - parentPaddingLeft;
			float parentHeight = YGNodeLayoutGetHeight(parent);
			if (parentHeight < height)
				height = parentHeight - YGNodeLayoutGetMargin(YogaNode, YGEdgeTop) - parentPaddingTop;
		}
			
		transform = glm::translate(transform, Vector3(left + parentLeft, top + parentTop, 0.f));


		Vector3 scale = Vector3(width, height, 0);
		transform = glm::scale(transform, scale);

		//Logger::Log("Left: " + std::to_string(left) + " Top:" + std::to_string(top));
		Renderer2D::UIShader->SetUniformMat4f("model", transform);

		Renderer2D::DrawRect();
	}
};