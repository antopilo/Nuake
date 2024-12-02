#pragma once
#include "src/Core/Core.h"
#include "src/Core/Maths.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "../../../Editor/src/Misc/InterfaceFonts.h"

#include "src/Resource/FontAwesome5.h"
#include "src/Rendering/Textures/Texture.h"

namespace Nuake
{
#define BEGIN_COLLAPSE_HEADER(names) \
    UIFont* names##_boldFont = new UIFont(Fonts::Bold); \
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f)); \
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 8.f)); \
    bool names##_Opened = ImGui::CollapsingHeader((std::string("  ") + #names).c_str()); \
    ImGui::PopStyleVar(); \
    delete names##_boldFont; \
    if (names##_Opened) { \

#define END_COLLAPSE_HEADER() \
} \
    ImGui::PopStyleVar(); \

	namespace UI
	{
		static uint32_t PrimaryCol = IM_COL32(97, 0, 255, 255);
		static uint32_t TextCol = IM_COL32(192, 192, 192, 255);;
		static uint32_t GrabCol = IM_COL32(97, 0, 255, 255);
		static ImVec2 ButtonPadding = ImVec2(16.0f, 8.0f);
		static ImVec2 IconButtonPadding = ImVec2(8.0f, 8.0f);

		void BeginWindow(const std::string& name);

		void EndWindow();

		bool PrimaryButton(const std::string& name, const Vector2& size = { 0, 0 }, Color color = Color(97.0f / 255.0f, 0, 1.0f, 1.0f));
		bool SecondaryButton(const std::string& name, const Vector2& size = { 0, 0 });

		bool IconButton(const std::string& icon);

		bool FloatSlider(const std::string& name, float& input, float min = 0.0f, float max = 1.0f, float speed = 0.01f);

		bool CheckBox(const std::string& name, bool& value);

		void ToggleButton(const char* str_id, bool* v);

		void Tooltip(const std::string& message);

		void DrawButtonImage(const Ref<Texture>& imageNormal, const Ref<Texture>& imageHovered, const Ref<Texture>& imagePressed,
			ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
			ImVec2 rectMin, ImVec2 rectMax);

		void DrawButtonImage(const Ref<Texture>& imageNormal, const Ref<Texture>& imageHovered, const Ref<Texture>& imagePressed,
			ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
			ImRect rectangle);

		void DrawButtonImage(const Ref<Texture>& image,
			ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
			ImVec2 rectMin, ImVec2 rectMax);

		void DrawButtonImage(const Ref<Texture>& image,
			ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
			ImRect rectangle);

		void DrawButtonImage(const Ref<Texture>& imageNormal, const Ref<Texture>& imageHovered, const Ref<Texture>& imagePressed,
			ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed);

		void DrawButtonImage(const Ref<Texture>& image, ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed);

		ImRect RectExpanded(const ImRect& rect, float x, float y);

		ImRect RectOffset(const ImRect& rect, float x, float y);

		ImRect RectOffset(const ImRect& rect, ImVec2 xy);

	}
}