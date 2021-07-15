#pragma once
#include "Node.h"

namespace Nuake
{
	struct TextStyle
	{
		Ref<Font> font;
		float fontSize;
		Color color;
	};

	class TextNode : public Node
	{
	public:
		std::string content = "Hello World!";
		TextStyle style;


		void SetTextStyle(TextStyle textStyle)
		{
			this->style = textStyle;
		}

		void CalculateSize()
		{

		}

		void Draw(float z)
		{
			Renderer2D::TextShader->Bind();

			Matrix4 transform = Matrix4(1.0f);
			float width = YGNodeLayoutGetWidth(this->YogaNode);
			float height = YGNodeLayoutGetHeight(this->YogaNode);
			float left = YGNodeLayoutGetLeft(this->YogaNode); //+ offset.x;
			float top = YGNodeLayoutGetTop(this->YogaNode);// +offset.y;

			float parentLeft = 0.0f;
			float parentTop = 0.0f;
			auto parent = YGNodeGetParent(YogaNode);

			while (parent)
			{
				parentLeft += YGNodeLayoutGetLeft(parent);
				parentTop += YGNodeLayoutGetTop(parent);
				parent = YGNodeGetParent(parent);
			}
			if (parent)
			{
				//parentLeft = YGNodeLayoutGetLeft(parent);
				//parentTop = YGNodeLayoutGetTop(parent);
				//float parentPaddingTop = YGNodeLayoutGetPadding(parent, YGEdgeTop);
				//float parentPaddingLeft = YGNodeLayoutGetPadding(parent, YGEdgeTop);
				// Overflow hidden.
				//float parentwidth = YGNodeLayoutGetWidth(parent);
				//if (parentwidth - YGNodeLayoutGetMargin(this->YogaNode, YGEdgeLeft) < width)
				//	width = parentwidth - parentPaddingLeft;
				//float parentHeight = YGNodeLayoutGetHeight(parent);
				//if (parentHeight < height)
				//	height = parentHeight - YGNodeLayoutGetMargin(this->YogaNode, YGEdgeTop) - parentPaddingTop;
			}



			transform = glm::translate(transform, Vector3(left + parentLeft, top + parentTop, 0.f));
			//transform = glm::scale(transform, Vector3(width, height, 1.0));
			//Renderer2D::UIShader->Bind();
			//Renderer2D::UIShader->SetUniformMat4f("model", transform);
			//Renderer2D::UIShader->SetUniform1f("u_BorderRadius", Border.Left.Value);
			//Renderer2D::UIShader->SetUniform2f("u_Size", width, height);

			//Renderer2D::DrawRect();

			//Logger::Log("Left: " + std::to_string(left) + " Top:" + std::to_string(top));

			Renderer2D::DrawString(content, style, transform);
			//Renderer2D::DrawString(content, style.font, Vector2(Position.Left, Position.Top), 2.0);
		}
	};
}
