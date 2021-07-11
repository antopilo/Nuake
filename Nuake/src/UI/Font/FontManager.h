#pragma once
#include <map>

#include "FontLoader.h"

namespace Nuake {
	class FontManager
	{
	private:
		static std::map <std::string, Ref<Font>> m_Fonts;

	public:
		static Ref<Font> GetFont(const std::string& font);
	};
}
