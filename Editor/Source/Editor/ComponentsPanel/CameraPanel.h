#pragma once
#include "ComponentPanel.h"
#include <Nuake/Scene/Components/CameraComponent.h>
#include "Nuake/FileSystem/FileSystem.h"

#include "Nuake/Core/Core.h"
#include "Nuake/Rendering/Vulkan/SceneViewport.h"
#include "Nuake/Rendering/Vulkan/DebugCmd.h"

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
            previewViewport->SetDebugName("CameraPreviewViewport");
            previewViewport->GetOnDebugDraw().AddStatic([&, componentPtr](DebugCmd& cmd)
            {
                    Matrix4 transform = Matrix4(1.0f);

                    auto& cam = cmd.GetScene()->m_EditorCamera;

                    Matrix4 initialTransform = Matrix4(1.0f);
                    initialTransform = glm::translate(initialTransform, cam->Translation);

                    Matrix4 gizmoTransform = initialTransform;
                    gizmoTransform = glm::inverse(componentPtr->CameraInstance->GetTransform());
                    gizmoTransform[3] = initialTransform[3];

                    auto view = componentPtr->CameraInstance->GetTransform();
                    auto proj = componentPtr->CameraInstance->GetPerspective();

                    static auto getGizmoScale = [](const Vector3& camPosition, const Nuake::Vector3& position) -> float
                    {
                        float distance = Distance(camPosition, position);

                        constexpr float ClosestDistance = 3.5f;
                        if (distance < ClosestDistance)
                        {
                            float fraction = distance / ClosestDistance;
                            return fraction;
                        }

                        return 1.0f;
                    };

                    Vector3 cameraPosition = componentPtr->CameraInstance->Translation;

                    const Vector3 gizmoSize = Vector3(Engine::GetProject()->Settings.GizmoSize);
                    gizmoTransform = glm::scale(gizmoTransform, gizmoSize * getGizmoScale(cameraPosition, initialTransform[3]));

                    cmd.DrawTexturedQuad(proj * view * gizmoTransform, TextureManager::Get()->GetTexture2("Resources/Gizmos/Camera.png"), Engine::GetProject()->Settings.PrimaryColor);
            });
            previewViewport->GetOnLineDraw().AddStatic([&, componentPtr](DebugCmd& cmd)
                {
                    auto& cam = cmd.GetScene()->m_EditorCamera;
                    Matrix4 initialTransform = Matrix4(1.0f);
                    initialTransform = glm::translate(initialTransform, cam->Translation);

                    const float aspectRatio = cam->AspectRatio;
                    const float fov = cam->Fov;

                    Matrix4 clampedProj = glm::perspectiveFov(glm::radians(fov), 9.0f * aspectRatio, 9.0f, 0.05f, 3.0f);
                    Matrix4 boxTransform = glm::translate(scene->GetCurrentCamera()->GetTransform(), Vector3(transform.GetGlobalTransform()[3])) * rotationMatrix * glm::inverse(clampedProj);
                    lineCmd.DrawBox(proj * boxTransform, Color(1, 0, 0, 1.0f), 1.5f, false);
                });

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