#include "FontManager.h"
#include "FontLoader.h"

namespace NuakeUI
{
	std::shared_ptr<Font> FontManager::GetFont(const std::string& font)
	{
		if (mFonts.find(font) == mFonts.end())
		{
			mFonts[font] = FontLoader::Get().LoadFont(font);
			return mFonts[font];
		}
		else
		{
			return mFonts[font];
		}
	}
}