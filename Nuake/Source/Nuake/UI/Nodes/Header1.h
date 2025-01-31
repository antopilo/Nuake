#pragma once
#include "Node.h"
#include "Nuake/UI/Font/Font.h"

#include <memory>
#include <string>

namespace NuakeUI
{
	class Header1;
	typedef std::shared_ptr<Header1> TextPtr;

	class Header1 : public Node
	{
	public:
		std::vector<std::string> Lines;
		std::shared_ptr<Font> mFont;

		static std::shared_ptr<Header1> New(const std::string& id, const std::string& text);
		Header1(const std::string& id, const std::string& text);
		~Header1() = default;

		void SetText(const std::string& text);
		std::string GetText() const;

		void Calculate() override;
		void Draw(int z) override;

		void SetFont(const std::string& fontPath);

		float CalculateWidth();
	};
}