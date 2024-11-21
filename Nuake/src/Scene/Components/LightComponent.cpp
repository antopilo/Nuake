#pragma once
//#include <imgui\imgui.h>
#include "LightComponent.h"
#include "src/Rendering/Renderer.h"

#include <glad/glad.h>
#include "src/Core/Core.h"
#include <src/Scene/Entities/ImGuiHelper.h>

namespace Nuake 
{
    LightComponent::LightComponent() :
        Color(1, 1, 1),
        Strength(10.0f),
        Direction(0, 1, 0)
    {
    }

    void LightComponent::SetCastShadows(bool toggle)
    {
        CastShadows = toggle;
        if (CastShadows)
        {
            if (Type == LightType::Directional || Type == LightType::Spot)
            {
				for (int i = 0; i < CSM_AMOUNT; i++)
				{
					m_Framebuffers[i] = CreateRef<FrameBuffer>(false, glm::vec2(4096, 4096));
					auto texture = CreateRef<Texture>(glm::vec2(4096, 4096), GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);
					texture->SetParameter(GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
					m_Framebuffers[i]->SetTexture(texture, GL_DEPTH_ATTACHMENT);
				}
            }
        }
        else 
        {
            for (int i = 0; i < CSM_AMOUNT; i++)
            {
                m_Framebuffers[i] = nullptr;
            }
        }
    }

    Matrix4 LightComponent::GetProjection()
    {
        if (Type == LightType::Directional)
        {
            return glm::ortho(-25.0f, 25.0f, -25.0f, 25.0f, -25.0f, 25.0f);
        }
        else if(Type == LightType::Spot)
        {
            return glm::perspectiveFov(glm::radians(OuterCutoff * 2.5f), 1.0f, 1.0f, 0.001f, 50.5f);
        }
    }

    Vector3 LightComponent::GetDirection()
    {
        return glm::normalize(Direction);
    }
}
