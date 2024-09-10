#pragma once
#include "Nodes/Canvas.h"
#include "Nodes/Text.h"
#include "Styles/StyleSheet.h"

#include <memory>

#include <Dependencies/NuakeRenderer/NuakeRenderer/NuakeRenderer.h>

namespace NuakeUI
{
	std::shared_ptr<Node> mSelectedNode;

	void DrawUI(std::shared_ptr<Node> node)
	{
		ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding | 
			ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

		// Highlight the selected node using a flag.
		if (mSelectedNode == node)
			base_flags |= ImGuiTreeNodeFlags_Selected;

		// Appends the classes of the node next to the name
		// Logic is only add [] if theres is a class and only add commans in between.
		std::string nodeTitle = node->GetID();
		nodeTitle = nodeTitle == "" ? node->GetType() : nodeTitle;
		const int classAmount = node->Classes.size();
		if (classAmount > 0)
		{
			nodeTitle += "[";
			for (int i = 0; i < classAmount; i++)
			{
				nodeTitle += node->Classes[i];

				if (i < classAmount - 1)
					nodeTitle += ", ";
			}
			nodeTitle += "]";
		}
		
		const bool nodeOpen = ImGui::TreeNodeEx(nodeTitle.c_str(), base_flags);

		// Select the if clicked
		if (ImGui::IsItemClicked())
		{
			mSelectedNode = node;
		}

		// Draw the rest of th nodes recursively.
		if (nodeOpen)
		{
			for (auto& c : node->GetChildrens())
				DrawUI(c);

			ImGui::TreePop();
		}
	}

	static void DrawNodeEditor()
	{
		if (!mSelectedNode)
		{
			ImGui::Text("No node selected.");
			return;
		}

		auto type = mSelectedNode->GetType();

		ImGui::SliderFloat("Width", &mSelectedNode->ComputedStyle.Width.value, 0.f, 1920.0f);
		ImGui::SliderFloat("Height", &mSelectedNode->ComputedStyle.Height.value, 0.0f, 1080.0f);
		ImGui::SliderFloat("Max Width", &mSelectedNode->ComputedStyle.MaxWidth.value, 0.f, 1920.0f);
		ImGui::SliderFloat("Max Height", &mSelectedNode->ComputedStyle.MaxHeight.value, 0.0f, 1080.0f);
		ImGui::SliderFloat("Min Width", &mSelectedNode->ComputedStyle.MinWidth.value, 0.f, 1920.0f);
		ImGui::SliderFloat("Min Height", &mSelectedNode->ComputedStyle.MinHeight.value, 0.0f, 1080.0f);

		ImGui::SliderFloat("Padding Left", &mSelectedNode->ComputedStyle.PaddingLeft.value, 0.f, 1920.0f);
		ImGui::SliderFloat("Padding Right", &mSelectedNode->ComputedStyle.PaddingRight.value, 0.0f, 1080.0f);
		ImGui::SliderFloat("Padding Top", &mSelectedNode->ComputedStyle.PaddingTop.value, 0.f, 1920.0f);
		ImGui::SliderFloat("Padding Bottom", &mSelectedNode->ComputedStyle.PaddingBottom.value, 0.0f, 1080.0f);

		ImGui::SliderFloat("Margin Left", &mSelectedNode->ComputedStyle.MarginLeft.value, 0.f, 1920.0f);
		ImGui::SliderFloat("Margin Right", &mSelectedNode->ComputedStyle.MarginRight.value, 0.0f, 1080.0f);
		ImGui::SliderFloat("Margin Top", &mSelectedNode->ComputedStyle.MarginTop.value, 0.f, 1920.0f);
		ImGui::SliderFloat("Margin Bottom", &mSelectedNode->ComputedStyle.MarginBottom.value, 0.0f, 1080.0f);

		ImGui::SliderFloat("Flex Basis", &mSelectedNode->ComputedStyle.FlexBasis, 0.f, 1920.0f);
		ImGui::SliderFloat("Flex Grow", &mSelectedNode->ComputedStyle.FlexGrow, 0.0f, 1080.0f);
		ImGui::SliderFloat("Flex Shrink", &mSelectedNode->ComputedStyle.FlexShrink, 0.f, 1920.0f);
		ImGui::SliderFloat("Font Size", &mSelectedNode->ComputedStyle.FontSize, 0.0f, 1080.0f);

		ImGui::ColorEdit4("Background Color", (float*) &mSelectedNode->ComputedStyle.BackgroundColor);
		ImGui::DragFloat("Border Size", &mSelectedNode->ComputedStyle.BorderSize, 1.f, 0.f);
		ImGui::ColorEdit4("Border Color", (float*)&mSelectedNode->ComputedStyle.BorderColor);
		ImGui::DragFloat("Font Size", &mSelectedNode->ComputedStyle.FontSize, 1.f, 0.f);
		ImGui::ColorEdit4("Font Color", (float*)&mSelectedNode->ComputedStyle.FontColor);
	}

	static void DrawInspector(std::shared_ptr<Canvas> canvas)
	{
		NuakeRenderer::BeginImGuiFrame();

		ImGui::ShowDemoWindow();

		if (ImGui::Begin("Inspector"))
		{
			if (ImGui::BeginTabBar("MyTabBar"))
			{
				if (ImGui::BeginTabItem("Tree"))
				{
					const float treeWidth = ImGui::GetWindowContentRegionWidth();
					const float availHeight = ImGui::GetContentRegionAvail().y;
					const ImVec2 size = ImVec2(treeWidth * 0.5f, availHeight);
					const ImVec2 size2 = ImVec2(treeWidth * 0.5f, availHeight);
					if (ImGui::BeginChild("Tree", size))
					{
						DrawUI(canvas->GetRoot());
					}
					ImGui::EndChild();

					ImGui::SameLine();

					if (ImGui::BeginChild("Editor", size2))
					{
						DrawNodeEditor();
					}
					ImGui::EndChild();
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("StyleSheet"))
				{
					if (ImGui::BeginChild("StyleSheetEditor", ImGui::GetContentRegionAvail()))
					{
						auto styleSheet = canvas->GetStyleSheet();

						int ri = 0;
						for (auto& r : styleSheet->Rules)
						{
							std::string imguiText = "";
							for (int s = 0; s < r.Selector.size(); s++)
							{
								std::string selectorText = "";

								auto& selector = r.Selector[s];
								auto type = selector.Type;
								if (type == StyleSelectorType::Id)
									selectorText += "#";
								else if (type == StyleSelectorType::Class)
									selectorText += ".";

								selectorText += selector.Value.c_str();

								if (s < r.Selector.size() - 1)
									selectorText += ", ";

								imguiText += selectorText;
							}

							imguiText += " { ";
							ImGui::Text(imguiText.c_str());

							ImGui::Indent(8.f);

							// Now the properties!
							int i = 0;
							for (auto& rule : r.Properties)
							{
								// Name
								std::string propName = "UnknownProperty";

								StyleProperties type = rule.first;
								if (type == StyleProperties::Width)					propName = "width: ";
								else if (type == StyleProperties::Height)			propName = "weight: ";
								else if (type == StyleProperties::MinWidth)			propName = "min-width: ";
								else if (type == StyleProperties::MinHeight)		propName = "min-height: ";
								else if (type == StyleProperties::MaxWidth)			propName = "max-width: ";
								else if (type == StyleProperties::MaxHeight)		propName = "max-height: ";
								else if (type == StyleProperties::BackgroundColor)	propName = "BackgroundColor: ";

								ImGui::Text(propName.c_str());
								ImGui::SameLine();

								std::string valueText = "";

								// value
								PropValue& value = rule.second;
								if (value.type == PropValueType::Percent)
								{
									valueText += std::to_string(value.value.Number);
									valueText += "\%;";
								}
								else if (value.type == PropValueType::Pixel)
								{
									valueText += std::to_string(value.value.Number);
									valueText += "px;";
								}
								else if (value.type == PropValueType::Color)
								{
									Color colorFloat = rule.second.value.Color / 255.f;
									ImGui::ColorEdit4(("##colorEdit" + std::to_string(ri) + propName + std::to_string(i)).c_str(), &colorFloat.r);
									rule.second.value.Color = colorFloat * 255.f;
								}
								else if (value.type == PropValueType::Auto)
								{
									valueText += "auto;";
								}
								i++;
								ImGui::Text(valueText.c_str());
							}
							ImGui::Indent(-8.f);
							ImGui::Text("}");
							ri++;
						}
					}
					ImGui::EndChild();
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
		}
		ImGui::End();

		NuakeRenderer::EndImGuiFrame();
	}
}