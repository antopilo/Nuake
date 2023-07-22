#include "FontManager.h"

namespace Nuake {
	std::map <std::string, Ref<Font>> FontManager::m_Fonts = std::map<std::string, Ref<Font>>();

	Ref<Font> FontManager::GetFont(const std::string& font)
	{
		// Fonts exists in memory
		if (m_Fonts.find(font) != m_Fonts.end())
		{
			return m_Fonts[font];
		}

		// Load font
		Ref<Font> newFont = FontLoader::LoadFont(font);
		if (newFont)
			return newFont;

		Logger::Log("Error: failed to load font " + font, "font", CRITICAL);
		return nullptr;
	}
}
