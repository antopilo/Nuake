#pragma once

#include <map>

enum Fonts {
	Normal, Bold, LargeBold, Icons, BigIcon, Title, SubTitle
};

struct ImFont;

// Help class that automatically push and pops itself when
// Reaching end of scope
class UIFont {
private:
	ImFont* ImGuiFont;
public:
	UIFont(Fonts font);
	~UIFont();
};

class FontManager {
private:
	static std::map < Fonts, ImFont*> mFonts;

public:
	static void LoadFonts();
	static ImFont* GetFont(Fonts font);
};