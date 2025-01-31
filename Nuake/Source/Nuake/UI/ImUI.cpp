#include "ImUI.h"
#include "Engine.h"
#include "Nuake/Resource/Project.h"

#include <imgui/imgui_internal.h>
#include "../../../NuakeEditor/Source/Editor/Misc/InterfaceFonts.h"

namespace Nuake 
{
	namespace UI
	{
		void BeginWindow(const std::string& name)
		{
			ImGui::Begin(name.c_str());
		}

		void EndWindow()
		{
			ImGui::End();
		}

		bool PrimaryButton(const std::string& name, const Vector2& size, Color color)
		{
			if (Nuake::Engine::GetProject())
			{
				color = Nuake::Engine::GetProject()->Settings.PrimaryColor;
			}

			ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(color.r * 255.0f, color.g * 255.0f, color.b * 255.0f, 255));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(color.r * 255.0f, color.g * 255.0f, color.b * 255.0f, 200));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(color.r * 255.0f, color.g * 255.0f, color.b * 255.0f, 255));

			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ButtonPadding);

			//UIFont boldFont(Bold);
			const bool buttonPressed = ImGui::Button(name.c_str(), ImVec2(size.x, size.y));

			ImGui::PopStyleColor(3);

			ImGui::PopStyleVar(2);

			return buttonPressed;
		}

		bool SecondaryButton(const std::string& name, const Vector2& size)
		{
			Color color = Color(97.0f / 255.0f, 0, 1.0f, 1.0f);
			if (Nuake::Engine::GetProject())
			{
				color = Nuake::Engine::GetProject()->Settings.PrimaryColor;
			}

			ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(color.r * 255.0f, color.g * 255.0f, color.b * 255.0f, 200));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(color.r * 255.0f, color.g * 255.0f, color.b * 255.0f, 255));
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

		void Tooltip(const std::string& message)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
			{
				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(450.0f);
				ImGui::TextUnformatted(message.c_str());
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();
			}

			ImGui::PopStyleVar(2);
		}

		ImRect RectExpanded(const ImRect& rect, float x, float y)
		{
			ImRect result = rect;
			result.Min.x -= x;
			result.Min.y -= y;
			result.Max.x += x;
			result.Max.y += y;
			return result;
		}

		ImRect RectOffset(const ImRect& rect, float x, float y)
		{
			ImRect result = rect;
			result.Min.x += x;
			result.Min.y += y;
			result.Max.x += x;
			result.Max.y += y;
			return result;
		}

		ImRect RectOffset(const ImRect& rect, ImVec2 xy)
		{
			return RectOffset(rect, xy.x, xy.y);
		}

		void DrawButtonImage(const Ref<VulkanImage>& imageNormal, const Ref<VulkanImage>& imageHovered, const Ref<VulkanImage>& imagePressed,
	ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
	ImVec2 rectMin, ImVec2 rectMax)
		{
			auto* drawList = ImGui::GetForegroundDrawList();
			if (ImGui::IsItemActive())
				drawList->AddImage((ImTextureID)imagePressed->GetImGuiDescriptorSet(), rectMin, rectMax, ImVec2(0, 0), ImVec2(1, 1), tintPressed);
			else if (ImGui::IsItemHovered())													   				 
				drawList->AddImage((ImTextureID)imagePressed->GetImGuiDescriptorSet(), rectMin, rectMax, ImVec2(0, 0), ImVec2(1, 1), tintHovered);
			else																				   				 
				drawList->AddImage((ImTextureID)imagePressed->GetImGuiDescriptorSet(), rectMin, rectMax, ImVec2(0, 0), ImVec2(1, 1), tintNormal);
		};

		void DrawButtonImage(const Ref<VulkanImage>& imageNormal, const Ref<VulkanImage>& imageHovered, const Ref<VulkanImage>& imagePressed,
			ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
			ImRect rectangle)
		{
			DrawButtonImage(imageNormal, imageHovered, imagePressed, tintNormal, tintHovered, tintPressed, rectangle.Min, rectangle.Max);
		};

		void DrawButtonImage(const Ref<VulkanImage>& image,
			ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
			ImVec2 rectMin, ImVec2 rectMax)
		{
			DrawButtonImage(image, image, image, tintNormal, tintHovered, tintPressed, rectMin, rectMax);
		};

		void DrawButtonImage(const Ref<VulkanImage>& image,
			ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
			ImRect rectangle)
		{
			DrawButtonImage(image, image, image, tintNormal, tintHovered, tintPressed, rectangle.Min, rectangle.Max);
		};


		void DrawButtonImage(const Ref<VulkanImage>& imageNormal, const Ref<VulkanImage>& imageHovered, const Ref<VulkanImage>& imagePressed,
			ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed)
		{
			DrawButtonImage(imageNormal, imageHovered, imagePressed, tintNormal, tintHovered, tintPressed, ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
		};

		void DrawButtonImage(const Ref<VulkanImage>& image,
			ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed)
		{
			DrawButtonImage(image, image, image, tintNormal, tintHovered, tintPressed, ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
		};

		bool Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size)
		{
			using namespace ImGui;
			ImGuiContext& g = *GImGui;
			ImGuiWindow* window = g.CurrentWindow;
			ImGuiID id = window->GetID("##Splitter");
			ImRect bb;
			bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
			bb.Max = bb.Min + CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f);
			return SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
		}
	}
}
