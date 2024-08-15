#include "ImUI.h"
#include <imgui/imgui_internal.h>

namespace Nuake {

	namespace UI {

		void BeginWindow(const std::string& name)
		{
			ImGui::Begin(name.c_str());
		}

		void EndWindow()
		{
			ImGui::End();
		}

		bool PrimaryButton(const std::string& name, const Vector2& size)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(97, 0, 255, 255));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(97, 0, 255, 200));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(97, 0, 255, 255));

			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ButtonPadding);

			UIFont boldFont(Bold);
			const bool buttonPressed = ImGui::Button(name.c_str(), ImVec2(size.x, size.y));

			ImGui::PopStyleColor(3);

			ImGui::PopStyleVar(2);

			return buttonPressed;
		}

		bool SecondaryButton(const std::string& name, const Vector2& size)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(97, 0, 255, 200));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(97, 0, 255, 255));
			ImGui::PushStyleColor(ImGuiCol_Border, PrimaryCol);

			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ButtonPadding);

			UIFont boldFont(Bold);
			const bool buttonPressed = ImGui::Button(name.c_str(), ImVec2(size.x, size.y));

			ImGui::PopStyleVar(3);

			ImGui::PopStyleColor(4);

			return buttonPressed;
		}

		bool IconButton(const std::string& icon)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, IconButtonPadding);

			const float height = ImGui::GetTextLineHeight() + ButtonPadding.y * 2.0f;
			const bool isPressed = ImGui::Button(icon.c_str(), ImVec2(height, height));

			ImGui::PopStyleVar(2);

			return isPressed;
		}

		bool FloatSlider(const std::string& name, float& input, float min, float max, float speed)
		{
			//ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, ImGui::GetStyle().ItemSpacing.y });
			//IconButton(ICON_FA_ANGLE_UP);
			//
			//ImGui::SameLine();

			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ButtonPadding);

			const bool isUsing = ImGui::DragFloat(("##" + name).c_str(), &input, speed, min, max);

			ImGui::PopStyleVar(2);

			//ImGui::PopStyleColor();

			return isUsing;
		}

		bool CheckBox(const std::string& name, bool& value)
		{
			const float height = ImGui::GetTextLineHeight() + ButtonPadding.y * 2.0f;

			if (!value)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(97, 0, 255, 200));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, PrimaryCol);
				ImGui::PushStyleColor(ImGuiCol_Border, PrimaryCol);

				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ButtonPadding);

				const bool buttonPressed = ImGui::Button(("##" + name).c_str(), ImVec2(height, height));

				ImGui::PopStyleVar(3);

				ImGui::PopStyleColor(4);

				if (buttonPressed)
				{
					value = !value;
				}
			}
			else
			{
				ImGui::PushStyleColor(ImGuiCol_Button, PrimaryCol);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, PrimaryCol);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, PrimaryCol);
				ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(97, 0, 255, 200));

				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ButtonPadding);

				const bool buttonPressed = ImGui::Button(("##" + name).c_str(), ImVec2(height, height));

				ImGui::PopStyleVar(3);

				ImGui::PopStyleColor(4);

				if (buttonPressed)
				{
					value = !value;
				}
			}

			return value;
		}

		void ToggleButton(const char* str_id, bool* v)
		{
			ImVec4* colors = ImGui::GetStyle().Colors;
			ImVec2 p = ImGui::GetCursorScreenPos();
			ImDrawList* draw_list = ImGui::GetWindowDrawList();

			float height = ImGui::GetFrameHeight();
			float width = height * 1.55f;
			float radius = height * 0.50f;

			ImGui::InvisibleButton(str_id, ImVec2(width, height));
			if (ImGui::IsItemClicked()) *v = !*v;
			ImGuiContext& gg = *GImGui;
			float ANIM_SPEED = 0.085f;
			if (gg.LastActiveId == gg.CurrentWindow->GetID(str_id))// && g.LastActiveIdTimer < ANIM_SPEED)
				float t_anim = ImSaturate(gg.LastActiveIdTimer / ANIM_SPEED);
			if (ImGui::IsItemHovered())
				draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), ImGui::GetColorU32(*v ? colors[ImGuiCol_ButtonActive] : ImVec4(0.78f, 0.78f, 0.78f, 1.0f)), height * 0.5f);
			else
				draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), ImGui::GetColorU32(*v ? colors[ImGuiCol_Button] : ImVec4(0.85f, 0.85f, 0.85f, 1.0f)), height * 0.50f);
			draw_list->AddCircleFilled(ImVec2(p.x + radius + (*v ? 1 : 0) * (width - radius * 2.0f), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
		}
	}
}