#pragma once
#include "InterfaceFonts.h"
#include <src/Vendors/imgui/imgui.h>
#include <src/Resource/FontAwesome5.h>

#include <src/Resource/StaticResources.h>
std::map<Fonts, ImFont*> FontManager::mFonts = std::map<Fonts, ImFont*>();

void FontManager::LoadFonts()
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true;

	using namespace Nuake::StaticResources;
	
	mFonts[Normal] = io.Fonts->AddFontFromMemoryTTF(Resources_Fonts_fa_solid_900_ttf, Resources_Fonts_fa_solid_900_ttf_len, 11.f, &icons_config, icons_ranges);
	
	
	ImFontConfig iconsConfigBold;
	icons_config.PixelSnapH = true;
	mFonts[Bold] = io.Fonts->AddFontFromMemoryTTF(Resources_Fonts_FiraMono_Bold_ttf, Resources_Fonts_FiraMono_Bold_ttf_len, 16.0);
	iconsConfigBold.MergeMode = true;
	io.Fonts->AddFontFromMemoryTTF(Resources_Fonts_fa_solid_900_ttf, Resources_Fonts_fa_solid_900_ttf_len, 16.f, &iconsConfigBold, icons_ranges);
	iconsConfigBold.MergeMode = false;

	mFonts[LargeBold] = io.Fonts->AddFontFromMemoryTTF(Resources_Fonts_FiraMono_Regular_ttf, Resources_Fonts_FiraMono_Regular_ttf_len, 32);

	mFonts[Title] = io.Fonts->AddFontFromMemoryTTF(Resources_Fonts_FiraMono_Bold_ttf, Resources_Fonts_FiraMono_Bold_ttf_len, 50.0);
	mFonts[SubTitle] = io.Fonts->AddFontFromMemoryTTF(Resources_Fonts_FiraMono_Regular_ttf, Resources_Fonts_FiraMono_Regular_ttf_len, 24.0);
	ImGui::GetIO().Fonts->AddFontDefault();
	mFonts[BigIcon] = io.Fonts->AddFontFromMemoryTTF(Resources_Fonts_fa_solid_900_ttf, Resources_Fonts_fa_solid_900_ttf_len, 42.0f, &icons_config, icons_ranges);
}

ImFont* FontManager::GetFont(Fonts font)
{
	return mFonts[font];
}

UIFont::UIFont(Fonts font)
{
	ImGuiFont = FontManager::GetFont(font);
	ImGui::PushFont(ImGuiFont);
}

UIFont::~UIFont()
{
	ImGui::PopFont();
}
