#pragma once
#include <vector>
#include "../Core/Core.h"
#include "yoga/Yoga.h"
#include "../Rendering/Renderer2D.h"
#include "../Core/Logger.h"
#include <src/UI/Styling/Style.h>

namespace Nuake
{
	enum class NodeType
	{
		Normal,
		Text
	};

	class Node
	{
	public:
		float ScrollAmount = 0.0f;

		int Depth;
		Ref<Node> Root;
		Ref<Node> Parent;
		std::string ID;

		std::vector<std::string> Groups;
		std::vector<Ref<Node>> Childrens = std::vector<Ref<Node>>();

		std::string OnClickSignature = "";

		NodeType Type = NodeType::Normal;

		YGNodeRef YogaNode;

		bool IsHover = false;
		bool IsClicked = false;

		Style NormalStyle;
		Style HoverStyle;

		Ref<Texture> BackgroundTexture;

		Node();

		void ApplyStyle(Ref<StyleGroup> stylegroup, StyleGroupSelector selector = StyleGroupSelector::Normal);

		bool IsPositionInside(Vector2 position);

		void AddGroup(const std::string& group)
		{
			Groups.push_back(group);
		}

		float GetWidth()
		{
			return YGNodeLayoutGetWidth(YogaNode);
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
			
		}

		void SetPadding()
		{
			
		}

		void SetBorder()
		{
			
		}

		void SetYogaLayout()
		{
			Style style = this->NormalStyle;
			if (IsHover)
				style = this->HoverStyle;

			switch (style.Width.mUnit)
			{
			case Layout::Unit::PIXEL:
				YGNodeStyleSetWidth(YogaNode, style.Width.Value);
				break;
			case Layout::Unit::PERCENT:
				YGNodeStyleSetWidthPercent(YogaNode, style.Width.Value);
				break;
			case Layout::Unit::AUTO:
				YGNodeStyleSetWidthAuto(YogaNode);
				break;
			}
			switch (style.Height.mUnit)
			{
			case Layout::Unit::PIXEL:
				YGNodeStyleSetHeight(YogaNode, style.Height.Value);
				break;
			case Layout::Unit::PERCENT:
				YGNodeStyleSetHeightPercent(YogaNode, style.Height.Value);
				break;
			case Layout::Unit::AUTO:
				YGNodeStyleSetHeightAuto(YogaNode);
				break;
			}

			switch (style.Margin.Left.mUnit)
			{
			case Layout::Unit::PIXEL:
				YGNodeStyleSetMargin(YogaNode, YGEdgeLeft, style.Margin.Left.Value);
				break;
			case Layout::Unit::PERCENT:
				YGNodeStyleSetMarginPercent(YogaNode, YGEdgeLeft, style.Margin.Left.Value);
				break;
			case Layout::Unit::AUTO:
				YGNodeStyleSetMarginAuto(YogaNode, YGEdgeLeft);
				break;
			}

			switch (style.Margin.Right.mUnit)
			{
			case Layout::Unit::PIXEL:
				YGNodeStyleSetMargin(YogaNode, YGEdgeRight, style.Margin.Right.Value);
				break;
			case Layout::Unit::PERCENT:
				YGNodeStyleSetMarginPercent(YogaNode, YGEdgeRight, style.Margin.Right.Value);
				break;
			case Layout::Unit::AUTO:
				YGNodeStyleSetMarginAuto(YogaNode, YGEdgeRight);
				break;
			}

			switch (style.Margin.Top.mUnit)
			{
			case Layout::Unit::PIXEL:
				YGNodeStyleSetMargin(YogaNode, YGEdgeTop, style.Margin.Top.Value);
				break;
			case Layout::Unit::PERCENT:
				YGNodeStyleSetMarginPercent(YogaNode, YGEdgeTop, style.Margin.Top.Value);
				break;
			case Layout::Unit::AUTO:
				YGNodeStyleSetMarginAuto(YogaNode, YGEdgeTop);
				break;
			}

			switch (style.Margin.Bottom.mUnit)
			{
			case Layout::Unit::PIXEL:
				YGNodeStyleSetMargin(YogaNode, YGEdgeBottom, style.Margin.Bottom.Value);
				break;
			case Layout::Unit::PERCENT:
				YGNodeStyleSetMarginPercent(YogaNode, YGEdgeBottom, style.Margin.Bottom.Value);
				break;
			case Layout::Unit::AUTO:
				YGNodeStyleSetMarginAuto(YogaNode, YGEdgeTop);
				break;
			}
			switch (style.Padding.Left.mUnit)
			{
			case Layout::Unit::PIXEL:
				YGNodeStyleSetPadding(YogaNode, YGEdgeLeft, style.Padding.Left.Value);
				break;
			case Layout::Unit::PERCENT:
				YGNodeStyleSetPaddingPercent(YogaNode, YGEdgeLeft, style.Padding.Left.Value);
				break;
			}

			switch (style.Padding.Right.mUnit)
			{
			case Layout::Unit::PIXEL:
				YGNodeStyleSetPadding(YogaNode, YGEdgeRight, style.Padding.Right.Value);
				break;
			case Layout::Unit::PERCENT:
				YGNodeStyleSetPaddingPercent(YogaNode, YGEdgeRight, style.Padding.Right.Value);
				break;
			}

			switch (style.Padding.Top.mUnit)
			{
			case Layout::Unit::PIXEL:
				YGNodeStyleSetPadding(YogaNode, YGEdgeTop, style.Padding.Top.Value);
				break;
			case Layout::Unit::PERCENT:
				YGNodeStyleSetPaddingPercent(YogaNode, YGEdgeTop, style.Padding.Top.Value);
				break;
			}

			switch (style.Padding.Bottom.mUnit)
			{
			case Layout::Unit::PIXEL:
				YGNodeStyleSetPadding(YogaNode, YGEdgeBottom, style.Padding.Bottom.Value);
				break;
			case Layout::Unit::PERCENT:
				YGNodeStyleSetPaddingPercent(YogaNode, YGEdgeBottom, style.Padding.Bottom.Value);
				break;
			}
			switch (style.Border.Left.mUnit)
			{
			case Layout::Unit::PIXEL:
				YGNodeStyleSetBorder(YogaNode, YGEdgeLeft, style.Border.Left.Value);
				break;
			}

			switch (style.Border.Right.mUnit)
			{
			case Layout::Unit::PIXEL:
				YGNodeStyleSetBorder(YogaNode, YGEdgeRight, style.Border.Right.Value);
				break;
			}

			switch (style.Border.Top.mUnit)
			{
			case Layout::Unit::PIXEL:
				YGNodeStyleSetBorder(YogaNode, YGEdgeTop, style.Border.Top.Value);
				break;
			}

			switch (style.Border.Bottom.mUnit)
			{
			case Layout::Unit::PIXEL:
				YGNodeStyleSetBorder(YogaNode, YGEdgeBottom, style.Border.Bottom.Value);
				break;
			}

			if (style.FlexWrap == Layout::FlexWrap::WRAP)
				YGNodeStyleSetFlexWrap(YogaNode, YGWrapWrap);
			else if (style.FlexWrap == Layout::FlexWrap::NO_WRAP)
				YGNodeStyleSetFlexWrap(YogaNode, YGWrapNoWrap);
			else if (style.FlexWrap == Layout::FlexWrap::WRAP_REVERSED)
				YGNodeStyleSetFlexWrap(YogaNode, YGWrapWrapReverse);

			if (style.FlexDirection == Layout::FlexDirection::ROW)
				YGNodeStyleSetFlexDirection(YogaNode, YGFlexDirectionRow);
			else if (style.FlexDirection == Layout::FlexDirection::ROW_REVERSED)
				YGNodeStyleSetFlexDirection(YogaNode, YGFlexDirectionRowReverse);
			else if (style.FlexDirection == Layout::FlexDirection::COLUMN)
				YGNodeStyleSetFlexDirection(YogaNode, YGFlexDirectionColumn);
			else if (style.FlexDirection == Layout::FlexDirection::COLUMN_REVERSED)
				YGNodeStyleSetFlexDirection(YogaNode, YGFlexDirectionColumnReverse);

			if (style.JustifyContent == Layout::JustifyContent::FLEX_START)
				YGNodeStyleSetJustifyContent(YogaNode, YGJustifyFlexStart);
			else if (style.JustifyContent == Layout::JustifyContent::FLEX_END)
				YGNodeStyleSetJustifyContent(YogaNode, YGJustifyFlexEnd);
			else if (style.JustifyContent == Layout::JustifyContent::CENTER)
				YGNodeStyleSetJustifyContent(YogaNode, YGJustifyCenter);
			else if (style.JustifyContent == Layout::JustifyContent::SPACE_BETWEEN)
				YGNodeStyleSetJustifyContent(YogaNode, YGJustifySpaceBetween);
			else if (style.JustifyContent == Layout::JustifyContent::SPACE_AROUND)
				YGNodeStyleSetJustifyContent(YogaNode, YGJustifySpaceAround);
			else if (style.JustifyContent == Layout::JustifyContent::SPACE_EVENLY)
				YGNodeStyleSetJustifyContent(YogaNode, YGJustifySpaceEvenly);

			if (style.AlignItems == Layout::AlignItems::FLEX_START)
				YGNodeStyleSetAlignItems(YogaNode, YGAlignFlexStart);
			else if (style.AlignItems == Layout::AlignItems::FLEX_END)
				YGNodeStyleSetAlignItems(YogaNode, YGAlignFlexEnd);
			else if (style.AlignItems == Layout::AlignItems::CENTER)
				YGNodeStyleSetAlignItems(YogaNode, YGAlignCenter);
			else if (style.AlignItems == Layout::AlignItems::BASELINE)
				YGNodeStyleSetAlignItems(YogaNode, YGAlignBaseline);
			else if (style.AlignItems == Layout::AlignItems::STRETCH)
				YGNodeStyleSetAlignItems(YogaNode, YGAlignStretch);
			//else if (AlignItems == Layout::AlignItems::AUTO)
			//	YGNodeStyleSetAlignItems(YogaNode, YGAlignAuto);

			if (style.AlignSelf == Layout::AlignItems::FLEX_START)
				YGNodeStyleSetAlignSelf(YogaNode, YGAlignFlexStart);
			else if (style.AlignSelf == Layout::AlignItems::FLEX_END)
				YGNodeStyleSetAlignSelf(YogaNode, YGAlignFlexEnd);
			else if (style.AlignSelf == Layout::AlignItems::CENTER)
				YGNodeStyleSetAlignSelf(YogaNode, YGAlignCenter);
			else if (style.AlignSelf == Layout::AlignItems::BASELINE)
				YGNodeStyleSetAlignSelf(YogaNode, YGAlignBaseline);
			else if (style.AlignSelf == Layout::AlignItems::STRETCH)
				YGNodeStyleSetAlignSelf(YogaNode, YGAlignStretch);
			else if (style.AlignSelf == Layout::AlignItems::AUTO)
				YGNodeStyleSetAlignSelf(YogaNode, YGAlignAuto);

			if (style.AlignContent == Layout::AlignContent::FLEX_START)
				YGNodeStyleSetAlignContent(YogaNode, YGAlignFlexStart);
			else if (style.AlignContent == Layout::AlignContent::FLEX_END)
				YGNodeStyleSetAlignContent(YogaNode, YGAlignFlexEnd);
			else if (style.AlignContent == Layout::AlignContent::CENTER)
				YGNodeStyleSetAlignContent(YogaNode, YGAlignCenter);
			else if (style.AlignContent == Layout::AlignContent::STRETCH)
				YGNodeStyleSetAlignContent(YogaNode, YGAlignStretch);
			else if (style.AlignContent == Layout::AlignContent::SPACE_BETWEEN)
				YGNodeStyleSetAlignContent(YogaNode, YGAlignSpaceBetween);
			else if (style.AlignContent == Layout::AlignContent::SPACE_AROUND)
				YGNodeStyleSetAlignContent(YogaNode, YGAlignSpaceAround);

			if (style.PositionType == Layout::PositionType::RELATIVE)
				YGNodeStyleSetPositionType(YogaNode, YGPositionTypeRelative);
			else if (style.PositionType == Layout::PositionType::ABSOLUTE)
				YGNodeStyleSetPositionType(YogaNode, YGPositionTypeAbsolute);
		}

		void Draw(float z)
		{
			Renderer2D::UIShader->Bind();
			Color color = this->NormalStyle.BackgroundColor;
			if(IsHover)
				color = this->HoverStyle.BackgroundColor;

			Renderer2D::UIShader->SetUniform4f("u_BackgroundColor",
				color.r / 255.f,
				color.g / 255.f,
				color.b / 255.f,
				color.a / 255.f
			);

			Matrix4 transform = Matrix4(1.0f);

			float width = YGNodeLayoutGetWidth(YogaNode);
			float height = YGNodeLayoutGetHeight(YogaNode);

			float scrollRange = height;


			float padding = YGNodeLayoutGetPadding(YogaNode, YGEdgeLeft);
			float left = YGNodeLayoutGetLeft(YogaNode); //+ offset.x;
			float top = YGNodeLayoutGetTop(YogaNode);// +offset.y;

			float parentLeft = 0.0f;
			float parentTop = 0.0f;
			float parentHeight = 0.0f;
			float parentWidth = 0.0f;

			auto parent = YGNodeGetParent(YogaNode);
			if (parent)
			{
				parentHeight = YGNodeLayoutGetHeight(parent);
				parentWidth = YGNodeLayoutGetWidth(parent);
			}

			while (parent)
			{
				parentLeft += YGNodeLayoutGetLeft(parent);
				parentTop += YGNodeLayoutGetTop(parent);
				parent = YGNodeGetParent(parent);
			}

			transform = glm::translate(transform, Vector3(left + parentLeft, top + parentTop, 0.f));

			//if (top + height > parentHeight)
			//{
			//	float heightDifference = parentHeight - (top + height);
			//	height = height + heightDifference;
			//}

			Vector3 scale = Vector3(width, height, 0);
			transform = glm::scale(transform, scale);
			
			Renderer2D::UIShader->SetUniformMat4f("model", transform);
			Renderer2D::UIShader->SetUniform1f("u_BorderRadius", NormalStyle.Border.Left.Value);
			Renderer2D::UIShader->SetUniform2f("u_Size", width, height);

			if (BackgroundTexture != nullptr)
			{
				BackgroundTexture->Bind(1);
				Renderer2D::UIShader->SetUniform1i("u_BackgroundTexture", 1);
				Renderer2D::UIShader->SetUniform1i("u_HasBackgroundTexture", 1);
			}
			else
				Renderer2D::UIShader->SetUniform1i("u_HasBackgroundTexture", 0);
				
			Renderer2D::DrawRect();
		}
	};
}