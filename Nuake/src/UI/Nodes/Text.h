#pragma once
#include "Node.h"
#include "../Font/Font.h"

#include <memory>
#include <string>

namespace NuakeUI
{
	class Text;
	typedef std::shared_ptr<Text> TextPtr;

	class Text : public Node
	{
	public:
		std::vector<std::string> Lines;
		std::shared_ptr<Font> mFont;

		static std::shared_ptr<Text> New(const std::string& id, const std::string& text);
		Text(const std::string& id, const std::string& text);
		~Text() = default;

		void SetText(const std::string& text);

		void Calculate() override;
		void UpdateInput(InputManager* manager) override {};
		void Draw(int z) override;

		float CalculateWidth();
	private:
	};
}