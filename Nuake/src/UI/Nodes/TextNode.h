#pragma once
#include "Node.h"


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
	

	void Draw()
	{
		//Renderer2D::DrawString(content, style.font, Vector2(Position.Left, Position.Top), 2.0);
	}
};