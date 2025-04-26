#pragma once
#include "ComponentPanel.h"
#include <Nuake/Scene/Components/CameraComponent.h>
#include "Nuake/FileSystem/FileSystem.h"

#include "Nuake/Core/Core.h"
#include "Nuake/Rendering/Vulkan/SceneViewport.h"

class CameraPanel 
{
public:
    static void Draw(Nuake::Entity& entity, entt::meta_any& componentInstance)
    {
        using namespace Nuake;
        Nuake::CameraComponent* componentPtr = componentInstance.try_cast<Nuake::CameraComponent>();
        static Ref<Nuake::Viewport> previewViewport;
        if (!previewViewport)
        {
            auto scene = entity.GetScene();
            auto& vkRenderer = Nuake::VkRenderer::Get();
            const UUID viewId = componentPtr->CameraInstance->ID;

            previewViewport = vkRenderer.CreateViewport(viewId, { 200, 200 });
            vkRenderer.RegisterSceneViewport(scene->Shared(), previewViewport->GetID());
        }
        if (componentPtr == nullptr)
        {
            return;
        }
        Nuake::CameraComponent& component = *componentPtr;

        UIFont* boldFont = new UIFont(Fonts::Bold);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 8.f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
        bool removed = false;
        bool headerOpened = ImGui::CollapsingHeader("CAMERA", ImGuiTreeNodeFlags_DefaultOpen);
        ImGui::PopStyleVar();
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::Selectable("Remove")) { removed = true; }
            ImGui::EndPopup();
        }
        if (removed)
        {
            entity.RemoveComponent<Nuake::CameraComponent>();
            ImGui::PopStyleVar();
            delete boldFont;
        }
        else if (headerOpened)
        {
            delete boldFont;

            auto size = ImVec2{ ImGui::GetContentRegionAvail().x, 200 };
            componentPtr->CameraInstance->OnWindowResize(size.x, size.y);
            previewViewport->QueueResize({ size.x, size.y });
            ImGui::BeginChild("##CameraPreview", size);
            {
                VkDescriptorSet textureDesc = previewViewport->GetRenderTarget()->GetImGuiDescriptorSet();
                ImGui::Image(textureDesc, ImGui::GetContentRegionAvail(), { 0, 1 }, { 1, 0 });
            }
            ImGui::EndChild();


            ImGui::PopStyleVar();
            ImGui::Indent();
            if (ImGui::BeginTable("Camera", 3, ImGuiTableFlags_SizingStretchProp))
            {
                ImGui::TableSetupColumn("name", 0, 0.25f);
                ImGui::TableSetupColumn("set", 0, 0.65f);
                ImGui::TableSetupColumn("reset", 0, 0.1f);

                ImGui::TableNextColumn();

                {
                    {
                        ImGui::Text("FOV");
                        ImGui::TableNextColumn();

                        ImGui::DragFloat("##Fov", &component.CameraInstance->Fov, 0.1f, 0.1f, 360.0f);
                        ImGui::TableNextColumn();
                        ComponentTableReset(component.CameraInstance->Fov, 88.0f);
                    }
                    ImGui::TableNextColumn();
                    {
                        ImGui::Text("Exposure");
                        ImGui::TableNextColumn();

                        ImGui::DragFloat("##Exposure", &component.CameraInstance->Exposure, 0.1f, 0.1f, 10.0f);
                        ImGui::TableNextColumn();
                        ComponentTableReset(component.CameraInstance->Fov, 1.0f);
                    }
                    ImGui::TableNextColumn();
                    {
                        ImGui::Text("Gamma");
                        ImGui::TableNextColumn();

                        ImGui::DragFloat("##Gamma", &component.CameraInstance->Gamma, 0.1f, 0.1f, 10.0f);
                        ImGui::TableNextColumn();
                        ComponentTableReset(component.CameraInstance->Fov, 2.2f);
                    }
                }
                ImGui::EndTable();
            }
            ImGui::Unindent();
        }
        else
        {
            ImGui::PopStyleVar();
            delete boldFont;
        }
        ImGui::PopStyleVar();
    }
};