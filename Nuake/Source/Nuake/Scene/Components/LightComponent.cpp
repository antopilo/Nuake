//#include <imgui/imgui.h>
#include "LightComponent.h"
#include "Nuake/Rendering/Renderer.h"

#include <glad/glad.h>
#include "Nuake/Core/Core.h"
#include "Nuake/Scene/Entities/ImGuiHelper.h"

namespace Nuake 
{
    float LightComponent::mCascadeSplitDepth[CSM_AMOUNT];
    LightComponent::LightComponent() :
        Color(1, 1, 1),
        Strength(10.0f),
        Direction(0, 1, 0)
		
    {
		m_LightViews = std::vector<LightView>(CSM_AMOUNT);
		for (int i = 0; i < CSM_AMOUNT; i++)
		{
            m_LightViews[i] = LightView{};
		}
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
                    m_ShadowMaps[i] = CreateRef<VulkanImage>(ImageFormat::D32F, Vector2{ 4096, 4096 }, ImageUsage::Depth);
					GPUResources::Get().AddTexture(m_ShadowMaps[i]);
				}
            }
        }
        else 
        {
            for (int i = 0; i < CSM_AMOUNT; i++)
            {
                // TODO: Delete old shadowmaps
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
