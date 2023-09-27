#include "ImUI.h"

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

			const float height = ImGui::GetTextLineHeight() + ButtonPadding.y * 2.0;
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
			const float height = ImGui::GetTextLineHeight() + ButtonPadding.y * 2.0;

			if (value)
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
					value = false;
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
					value = true;
				}
			}

			return value;
		}
	}
}