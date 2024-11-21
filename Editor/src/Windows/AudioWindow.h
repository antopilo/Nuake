#pragma once
#include "imgui/imgui.h"

class AudioWindow
{
public:
	AudioWindow() = default;
	~AudioWindow() = default;

	float dummyVolume = 1.0f;
	void Draw()
	{
		//constexpr int numBus = 6;
		//if(ImGui::Begin("Audio mixer"))
		//{
		//	for (int i = 0; i < numBus; i++)
		//	{
		//		const std::string busName = "Bus " + std::to_string(i);
		//		//ImGui::BeginTable(busName.c_str(), 1, 0, ImVec2(300, ImGui::GetContentRegionAvail().y));
		//		//
		//		//ImGui::TableSetupColumn(busName.c_str(), 0, 1.0f);
		//		//ImGui::TableNextColumn ();
		//		//ImGui::TableHeader(busName.c_str());
		//		//ImGui::TableNextColumn();
		//		//
		//		//const float height = ImGui::GetContentRegionAvail().y - 50;
		//		//const std::string id = "##Volume" + std::to_string(i);
		//		//ImGui::VSliderFloat(id.c_str(), ImVec2(50, height), &dummyVolume, -60.0f, 6.0f, "%.3dB", ImGuiSliderFlags_Logarithmic);
		//		//ImGui::TableNextColumn();
		//		//ImGui::EndTable();
		//	}
		//}
		
		//ImGui::End();
	}
};