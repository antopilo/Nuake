#include "Text.h"

#include "../Renderer.h"

#include "Nuake/Rendering/Renderer.h"
#include "Nuake/UI/Font/FontManager.h"

#include <sstream>

namespace NuakeUI
{
	std::shared_ptr<Text> Text::New(const std::string& id, const std::string& text)
	{
		return std::make_shared<Text>(id, text);
	}

	Text::Text(const std::string& id, const std::string& text)
	{
		Type = "text";
		ID = id;
		mNode = YGNodeNew();
		mFont = Renderer::Get().mDefaultFont;

		SetText(text);
		float height = ((mFont->LineHeight) / 32.f) * (ComputedStyle.FontSize * 2.0)  * Lines.size();
		YGNodeStyleSetHeight(mNode, height);
		YGNodeStyleSetMinHeight(mNode, height);
		YGNodeStyleSetMinWidth(mNode, CalculateWidth());
		YGNodeStyleSetWidthPercent(mNode, 100.f);
	}

	void Text::SetText(const std::string& text)
	{
		Lines.clear();

		// Split into lines
		auto ss = std::stringstream{ text };
		for (std::string line; std::getline(ss, line, '\n');)
			Lines.push_back(line);

		if(std::size(Lines) > 0)
			Calculate();
	}

	std::string Text::GetText() const
	{
		std::string result = "";
		for (const auto& line : Lines)
		{
			result += line + "\n";
		}

		return result;
	}

	void Text::Draw(int z)
	{
		const float width = YGNodeLayoutGetWidth(mNode);
		const float height = YGNodeLayoutGetHeight(mNode);
		ComputedSize = { width, height };

		float x = YGNodeLayoutGetLeft(mNode); 
		float y = YGNodeLayoutGetTop(mNode);

		// Centers the text in the line height.
		y += (ComputedStyle.FontFamily->LineHeight / 64.0f) * (ComputedStyle.FontSize * 2.0) / 2.0f;

		x += YGNodeLayoutGetPadding(mNode, YGEdgeLeft);
		y += YGNodeLayoutGetPadding(mNode, YGEdgeTop);

		auto parent = Parent;
		bool hasParent = parent != nullptr;
		if (hasParent)
		{
			x += parent->ComputedPosition.x;
			y += parent->ComputedPosition.y - parent->GetScroll();
		}

		Vector3 position = Vector3(x, y, z);
		ComputedPosition = position;

		if (ComputedStyle.TextAlign == TextAlignType::Center)
		{	// We center the text horizontally.
			position.x += (width / 2.0f) - CalculateWidth() / 2.0f;
		}
		else if (ComputedStyle.TextAlign == TextAlignType::Right)
		{	// Aligns the line of the left
			position.x += width - CalculateWidth(); 
		}

		// Scissor the parent bounding box.
		bool hideOverflow = hasParent && Parent->ComputedStyle.Overflow == OverflowType::Hidden;
		if (hideOverflow)
		{
			//glEnable(GL_SCISSOR_TEST);
			int clipX = (int)Parent->ComputedPosition.x;
			int clipY = (1080 - (int)Parent->ComputedPosition.y - (int)Parent->ComputedSize.y);
			int clipWidth = (int)Parent->ComputedSize.x;
			int clipHeight = (int)Parent->ComputedSize.y;
			//glScissor(clipX, clipY, clipWidth, clipHeight);
		}

		const float lineYOffset = (ComputedStyle.FontFamily->LineHeight / 32.0f) * (ComputedStyle.FontSize * 2.0);
		// Draw each line and offset the Y of the position by the line height.
		for(int i = 0; i < Lines.size(); i++)
		{
			// Draw the first line
			Renderer::Get().DrawString(Lines[i], ComputedStyle, ComputedStyle.FontFamily, position);
			// Update the Y position to the next line.
			position.y += lineYOffset;
		}

		// Disable scissoring.
		if (hideOverflow){}
			//glDisable(GL_SCISSOR_TEST);
	}

	float Text::CalculateWidth()
	{
		// If theres no text, then assume it's 0;
		if (Lines.size() == 0) return 0.f;

		const float fontSize = ComputedStyle.FontSize * 2.0 / 64.f;
		
		// Find the largest line.
		float maxWidth = 0.f;
		for (auto& l : Lines)
		{
			float textWidth = 0.f;
			// Iterate over each character and add up the advance.
			for (char const& c : l)
			{
				Char letter = ComputedStyle.FontFamily->GetChar((int)c);
				textWidth += (letter.Advance * fontSize);
			}

			if (textWidth > maxWidth)
				maxWidth = textWidth;
		}

		// Scale the width by the font size.
		return maxWidth;
	}

	void Text::Calculate()
	{
		const float halfLineHeight = ComputedStyle.FontFamily->LineHeight / 32.f;
		const float linesHeight = Lines.size() * ComputedStyle.FontSize * 2.0;
		YGNodeStyleSetHeight(mNode, halfLineHeight * linesHeight);
		YGNodeStyleSetWidth(mNode, CalculateWidth());
	}
}