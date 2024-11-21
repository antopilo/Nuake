#pragma once
#include "Font.h"

#include <string>
#include <map>
#include <memory>

namespace NuakeUI
{
	class FontManager
	{
	private:
		std::map<std::string, std::shared_ptr<Font>> mFonts;
	public:
		static FontManager& Get() {
			static FontManager fontManager;
			return fontManager;
		}

		FontManager() {
			mFonts = std::map<std::string, std::shared_ptr<Font>>();
		}

		std::shared_ptr<Font> GetFont(const std::string& font);
	};
}