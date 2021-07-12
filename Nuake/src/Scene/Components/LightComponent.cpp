#pragma once
//#include <imgui\imgui.h>
#include "LightComponent.h"
#include "src/Rendering/Renderer.h"

#include <GL\glew.h>
#include "src/Core/Core.h"
#include <src/Scene/Entities/ImGuiHelper.h>

namespace Nuake {
    LightComponent::LightComponent()
    {
        Color = glm::vec3(1, 1, 1);
        Strength = 10.0f;
        Direction = glm::vec3(0, -1, 0);


        //m_Framebuffers = std::vector<Ref<FrameBuffer>>();
        //mViewProjections = std::vector<glm::mat4>();
        //mCascadeSplitDepth = std::vector<float>();
        //mCascadeSplits = std::vector<float>();
        // Framebuffer used for shadow mapping.

        //for (int i = 0; i < 4; i++)
        //{
        //    m_Framebuffers[i] = CreateRef<FrameBuffer>(false, glm::vec2(4096, 4096));
        //    m_Framebuffers[i]->SetTexture(CreateRef<Texture>(glm::vec2(4096, 4096), GL_DEPTH_COMPONENT), GL_DEPTH_ATTACHMENT);
        //}

    }

    void LightComponent::SetCastShadows(bool toggle)
    {
        CastShadows = toggle;
        if (CastShadows)
        {
            for (int i = 0; i < 4; i++)
            {
                m_Framebuffers[i] = CreateRef<FrameBuffer>(false, glm::vec2(4096, 4096));
                m_Framebuffers[i]->SetTexture(CreateRef<Texture>(glm::vec2(4096, 4096), GL_DEPTH_COMPONENT), GL_DEPTH_ATTACHMENT);
            }
        }
        else {
            for (int i = 0; i < 4; i++)
            {
                m_Framebuffers[i] = nullptr;
            }
        }
    }

    glm::mat4 LightComponent::GetProjection()
    {
        return glm::ortho(-25.0f, 25.0f, -25.0f, 25.0f, -25.0f, 25.0f);
    }

    void LightComponent::SetDirection(glm::vec3 dir)
    {

    }

    glm::vec3 LightComponent::GetDirection()
    {
        //glm::mat4 start = glm::mat4(1.0f);
        //glm::vec3 defaultDirection(0, 0, 1); // forward
        //
        //start = glm::rotate(start, glm::radians(Direction.x), glm::vec3(1, 0, 0));
        //start = glm::rotate(start, glm::radians(Direction.y), glm::vec3(0, 1, 0));
        //start = glm::rotate(start, glm::radians(Direction.z), glm::vec3(0, 0, 1));

        //return glm::vec3(start * glm::vec4(defaultDirection, 1.0f));
        return Direction;
    }


    void LightComponent::BeginDrawShadow()
    {
        Renderer::m_ShadowmapShader->Bind();
        //m_Framebuffer->Bind();

        // Render scene...

    }

    void LightComponent::EndDrawShadow()
    {
        //m_Framebuffer->Unbind();
    }

    void LightComponent::DrawShadow()
    {
        if (Type != Directional)
            return;

        Renderer::m_ShadowmapShader->Bind();
    }

    void LightComponent::Draw(TransformComponent transformComponent, Ref<Camera> cam)
    {
        Renderer::RegisterLight(transformComponent, *this, cam);
    }

    void LightComponent::DrawDeferred(TransformComponent transformComponent, Camera* cam)
    {
        Renderer::RegisterDeferredLight(transformComponent, *this, cam);
    }

    void LightComponent::DrawEditor() {
        ImGui::TextColored(ImGui::GetStyleColorVec4(1), "Light properties");
        ImGui::ColorEdit3("Light Color", &Color.r);
        ImGui::SliderFloat("Strength", &Strength, 0.0f, 50.0f);
        bool before = CastShadows;
        ImGui::Checkbox("Cast shadows", &CastShadows);

        if (CastShadows && before == false)
            SetCastShadows(true);

        const char* types[] = { "Directional", "Point", "Spot" };
        static const char* current_item = types[Type];

        if (ImGui::BeginCombo("Type", current_item)) // The second parameter is the label previewed before opening the combo.
        {
            for (int n = 0; n < IM_ARRAYSIZE(types); n++)
            {
                bool is_selected = (current_item == types[n]); // You can store your selection however you want, outside or inside your objects
                if (ImGui::Selectable(types[n], is_selected)) {
                    current_item = types[n];
                    Type = (LightType)n;
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
            }
            ImGui::EndCombo();
        }

        if (Type == Directional) {
            ImGui::Checkbox("Sync with sky", &SyncDirectionWithSky);
            ImGui::Checkbox("Volumetric?", &IsVolumetric);
            ImGuiHelper::DrawVec3("Direction", &Direction);
        }

        //if (Type == 1) {
        //    ImGui::SliderFloat("Attenuation", &Attenuation, 0.0f, 1.0f);
        //    ImGui::SliderFloat("Linear attenuation", &LinearAttenuation, 0.0f, 1.0f);
        //    ImGui::SliderFloat("Quadratic attenuation", &QuadraticAttenuation, 0.0f, 1.0f);
        //}

        //Direction = glm::normalize(Direction);
    }
}
