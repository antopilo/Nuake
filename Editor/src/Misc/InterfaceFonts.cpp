#pragma once
#include "InterfaceFonts.h"
#include <src/Vendors/imgui/imgui.h>
#include <src/Resource/FontAwesome5.h>

std::map<Fonts, ImFont*> FontManager::mFonts = std::map<Fonts, ImFont*>();

void FontManager::LoadFonts()
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true;

	mFonts[Normal] = io.Fonts->AddFontFromFileTTF("resources/Fonts/fa-solid-900.ttf", 11.0f, &icons_config, icons_ranges);
	mFonts[Bold] = io.Fonts->AddFontFromFileTTF("resources/Fonts/OpenSans-Bold.ttf", 16.0);
	mFonts[LargeBold] = io.Fonts->AddFontFromFileTTF("resources/Fonts/OpenSans-Regular.ttf", 32);
	mFonts[Title] = io.Fonts->AddFontFromFileTTF("resources/Fonts/OpenSans-Bold.ttf", 50.0);
	mFonts[SubTitle] = io.Fonts->AddFontFromFileTTF("resources/Fonts/OpenSans-Regular.ttf", 24.0);
	ImGui::GetIO().Fonts->AddFontDefault();
	icons_config.MergeMode = true;
	mFonts[BigIcon] = io.Fonts->AddFontFromFileTTF("resources/Fonts/fa-solid-900.ttf", 42.0f, &icons_config, icons_ranges);
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
