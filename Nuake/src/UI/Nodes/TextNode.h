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
		TextStyle textStyle;


		void SetTextStyle(TextStyle textStyle)
		{
			this->textStyle = textStyle;
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

			float maxWidth = 0; 
			if (parent)
				maxWidth = YGNodeLayoutGetWidth(parent);

			while (parent)
			{
				parentLeft += YGNodeLayoutGetLeft(parent);
				parentTop += YGNodeLayoutGetTop(parent);
				parent = YGNodeGetParent(parent);
			}
			transform = glm::translate(transform, Vector3(left + parentLeft, top + parentTop, 0.f));
			Renderer2D::DrawString(content, textStyle, transform);
		}
	};
}
