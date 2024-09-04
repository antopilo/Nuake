#pragma once
#include "ComponentPanel.h"

#include "src/FileSystem/FileSystem.h"
#include <src/Core/Maths.h>
#include <src/Scene/Components/AudioEmitterComponent.h>
#include <src/Scene/Entities/ImGuiHelper.h>


class AudioEmitterPanel : ComponentPanel
{
public:
    AudioEmitterPanel() = default;
    ~AudioEmitterPanel() = default;

	void Draw(Nuake::Entity entity) override
	{
		using namespace Nuake;

		if (!entity.HasComponent<AudioEmitterComponent>())
			return;

		auto& component = entity.GetComponent<AudioEmitterComponent>();
		BeginComponentTable(AUDIO EMITTER, AudioEmitterComponent);
		{
			{
                ImGui::Text("Audio File");
                ImGui::TableNextColumn();

                std::string path = component.FilePath;
                ImGui::Button(component.FilePath.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0));
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_AudioFile"))
                    {
                        char* file = (char*)payload->Data;
                        std::string fullPath = std::string(file, 256);
                        path = Nuake::FileSystem::AbsoluteToRelative(fullPath);
                        component.FilePath = path;
                    }
                    ImGui::EndDragDropTarget();
                }

                ImGui::TableNextColumn();

                ComponentTableReset(component.FilePath, "");
			}
            ImGui::TableNextColumn();
            {
                ImGui::Text("Playing");
                ImGui::TableNextColumn();

                UI::ToggleButton("##Player", &component.IsPlaying);
                //ImGui::Checkbox("##Playing", &component.IsPlaying);
                ImGui::TableNextColumn();

                ComponentTableReset(component.IsPlaying, false);
            }
            ImGui::TableNextColumn();
            {
                ImGui::Text("Loop");
                ImGui::TableNextColumn();

                ImGui::Checkbox("##Loop", &component.Loop);
                ImGui::TableNextColumn();

                ComponentTableReset(component.Loop, false);
            }
            ImGui::TableNextColumn();
            {
                ImGui::Text("Volume");
                ImGui::TableNextColumn();

                ImGui::DragFloat("##Volume", &component.Volume, 0.001f, 0.0f, 2.0f);
                ImGui::TableNextColumn();

                ComponentTableReset(component.Volume, 1.0f);
            }
            ImGui::TableNextColumn();
            {
                ImGui::Text("Playback Speed");
                ImGui::TableNextColumn();

                ImGui::DragFloat("##PlaybackSpeed", &component.PlaybackSpeed, 0.01f, 0.0001f);
                ImGui::TableNextColumn();

                ComponentTableReset(component.PlaybackSpeed, 1.0f);
            }
            ImGui::TableNextColumn();
            {
                ImGui::Text("Pan");
                ImGui::TableNextColumn();

                ImGui::DragFloat("##Pan", &component.Pan, 0.01f, -1.0f, 1.0f);
                ImGui::TableNextColumn();

                ComponentTableReset(component.Pan, 0.0f);
            }

            ImGui::TableNextColumn();
            {
                ImGui::Text("Spatialized");
                ImGui::TableNextColumn();

                ImGui::Checkbox("##Spatialized", &component.Spatialized);
                ImGui::TableNextColumn();

                ComponentTableReset(component.Spatialized, false);
            }

            if (component.Spatialized)
            {
                ImGui::TableNextColumn();
                {
                    ImGui::Text("Min Distance");
                    ImGui::TableNextColumn();

                    ImGui::DragFloat("##minDistance", &component.MinDistance, 0.001f, 0.0f);
                    ImGui::TableNextColumn();

                    ComponentTableReset(component.MinDistance, 1.0f);
                }
                ImGui::TableNextColumn();
                {
                    ImGui::Text("Max Distance");
                    ImGui::TableNextColumn();

                    ImGui::DragFloat("##maxDistance", &component.MaxDistance, 0.001f, 0.0f);
                    ImGui::TableNextColumn();

                    ComponentTableReset(component.MaxDistance, 10.0f);
                }
                ImGui::TableNextColumn();
                {
                    ImGui::Text("Attenuation Factor");
                    ImGui::TableNextColumn();

                    ImGui::DragFloat("##attenuationFactor", &component.AttenuationFactor, 0.001f, 0.0f);
                    ImGui::TableNextColumn();

                    ComponentTableReset(component.AttenuationFactor, 1.0f);
                }
            }
		}
		EndComponentTable();
	}
};