#pragma once

#include "Component.h"

#include "TransformComponent.h"
#include "Nuake/Rendering/Camera.h"
#include "VisibilityComponent.h"
#include "Nuake/Resource/Serializable.h"
#include "Nuake/Rendering/Buffers/Framebuffer.h"
#include "Nuake/Rendering/Vulkan/VulkanImage/VulkanImage.h"

#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/matrix_clip_space.hpp>

namespace Nuake
{
    enum LightType 
    {
        Directional, Point, Spot
    };

    struct LightView
    {
		UUID CameraID;

        Matrix4 View;
        Matrix4 Proj;
    };

    const int CSM_AMOUNT = 4;
    class LightComponent : public Component
    {
        NUAKECOMPONENT(LightComponent, "Light")

    public:
        LightType Type = Point;
        Vector3 Direction;
        float Cutoff = 12.5f;
        float OuterCutoff = 20.0f;
        Vector3 Color;

        bool IsVolumetric = false;
        float Strength;

        bool SyncDirectionWithSky = false;
        bool CastShadows = false;
		Ref<VulkanImage> m_ShadowMaps[CSM_AMOUNT];
        Matrix4 mViewProjections[CSM_AMOUNT];
        std::vector<LightView> m_LightViews;
        static float mCascadeSplitDepth[CSM_AMOUNT];
        float mCascadeSplits[CSM_AMOUNT];

    public:

        LightComponent();
        ~LightComponent() = default;

        std::vector<UUID> LightMapIDs = std::vector<UUID>();
        void SetCastShadows(bool toggle);

        Matrix4 GetProjection();
        Vector3 GetDirection();

        void CalculateViewProjection(glm::mat4& view, const glm::mat4& projection)
        {
            Matrix4 viewProjection = projection * view;
            Matrix4 inverseViewProjection = glm::inverse(viewProjection);

            // TODO: Automate this
            const float nearClip = 0.01f;
            const float farClip = 200.0f;
            const float clipRange = farClip - nearClip;

            const float mCascadeNearPlaneOffset = -100.0f;
            const float mCascadeFarPlaneOffset = 100.0;

            // Calculate the optimal cascade distances
            const float minZ = nearClip;
            const float maxZ = nearClip + clipRange;
            const float range = maxZ - minZ;
            const float ratio = maxZ / minZ;
            for (int i = 0; i < CSM_AMOUNT; i++)
            {
                const float p = (i + 1) / static_cast<float>(4);
                const float log = minZ * glm::pow(ratio, p);
                const float uniform = minZ + range * p;
                const float d = 0.91f * (log - uniform) + uniform;
                mCascadeSplits[i] = (d - nearClip) / clipRange;
            }

            //mCascadeSplits[0] = 0.01f;

            float lastSplitDist = 0.0f;
            // Calculate Orthographic Projection matrix for each cascade
            for (int cascade = 0; cascade < CSM_AMOUNT; cascade++)
            {
                float splitDist = mCascadeSplits[cascade];
                Vector4 frustumCorners[8] =
                {
                    //Near face
                    {  1.0f, -1.0f,  1.0f, 1.0f },
                    { -1.0f, -1.0f,  1.0f, 1.0f },
                    {  1.0f,  1.0f,  1.0f, 1.0f },
                    { -1.0f,  1.0f,  1.0f, 1.0f },

                    // Far face (z = 0.0)
                    {  1.0f, -1.0f,  0.0f, 1.0f },
                    { -1.0f, -1.0f,  0.0f, 1.0f },
                    {  1.0f,  1.0f,  0.0f, 1.0f },
                    { -1.0f,  1.0f,  0.0f, 1.0f },
                };

                // Project frustum corners into world space from clip space
                for (int i = 0; i < 8; i++)
                {
                    Vector4 invCorner = inverseViewProjection * frustumCorners[i];
                    frustumCorners[i] = invCorner / invCorner.w;
                }

                for (int i = 0; i < CSM_AMOUNT; i++)
                {
                    Vector4 dist = frustumCorners[i + 4] - frustumCorners[i];
                    frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
                    frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
                }

                // Get frustum center
                Vector3 frustumCenter = Vector3(0.0f);
                for (int i = 0; i < 8; i++)
                {
                    frustumCenter += Vector3(frustumCorners[i]);
                }
                frustumCenter /= 8.0f;

                // Get the minimum and maximum extents
                float radius = 0.0f;
                for (int i = 0; i < 8; i++)
                {
                    float distance = glm::length(Vector3(frustumCorners[i]) - frustumCenter);
                    radius = glm::max(radius, distance);
                }
                radius = std::ceil(radius * 16.0f) / 16.0f;
                Vector3 maxExtents = Vector3(radius);
                Vector3 minExtents = -maxExtents;

                // Calculate the view and projection matrix
                Vector3 lightDir = this->Direction;
                Matrix4 lightViewMatrix = glm::lookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter, Vector3(0.0f, 1.0, 0.0f));
                Matrix4 lightProjectionMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f + mCascadeNearPlaneOffset, maxExtents.z - minExtents.z + mCascadeFarPlaneOffset);
                 
                // Offset to texel space to avoid shimmering ->(https://stackoverflow.com/questions/33499053/cascaded-shadow-map-shimmering)
                Matrix4 shadowMatrix = lightProjectionMatrix * lightViewMatrix;
                const float ShadowMapResolution = 4096;
                Vector4 shadowOrigin = (shadowMatrix * Vector4(0.0f, 0.0f, 0.0f, 1.0f)) * ShadowMapResolution / 2.0f;
                Vector4 roundedOrigin = glm::round(shadowOrigin);
                Vector4 roundOffset = roundedOrigin - shadowOrigin;
                roundOffset = roundOffset * 2.0f / ShadowMapResolution;
                roundOffset.z = 0.0f;
                roundOffset.w = 0.0f;
                lightProjectionMatrix[3] += roundOffset;

                m_LightViews[cascade].View = lightViewMatrix;
                m_LightViews[cascade].Proj = lightProjectionMatrix;

                // Store SplitDistance and ViewProjection-Matrix
                mCascadeSplitDepth[cascade] = (nearClip + splitDist * clipRange) * 1.0f;
                mViewProjections[cascade] = shadowMatrix;
                lastSplitDist = mCascadeSplits[cascade];
            }
        }

        json Serialize()
        {
            BEGIN_SERIALIZE();
            SERIALIZE_VAL(Type);
            SERIALIZE_VEC3(Direction);
            SERIALIZE_VEC3(Color);
            SERIALIZE_VAL(IsVolumetric);
            SERIALIZE_VAL(Strength);
            SERIALIZE_VAL(SyncDirectionWithSky);
            SERIALIZE_VAL(CastShadows);
            SERIALIZE_VAL(Cutoff);
            SERIALIZE_VAL(OuterCutoff);
            END_SERIALIZE();
        }

        bool Deserialize(const json& j)
        {
            if (j.contains("Type"))
                Type = static_cast<LightType>(j["Type"]);
            if (j.contains("IsVolumetric"))
                IsVolumetric = j["IsVolumetric"];
            if (j.contains("Color"))
                Color = Vector3(j["Color"]["x"], j["Color"]["y"], j["Color"]["z"]);
            if (j.contains("Strength"))
                Strength = j["Strength"];
            if (j.contains("SyncDirectionWithSky"))
                SyncDirectionWithSky = j["SyncDirectionWithSky"];
            if (j.contains("CastShadows"))
                SetCastShadows(j["CastShadows"]);
            if (j.contains("Direction"))
            {
                float x = j["Direction"]["x"];
                float y = j["Direction"]["y"];
                float z = j["Direction"]["z"];
                this->Direction = Vector3(x, y, z);
            }

            DESERIALIZE_VAL(Cutoff);
            DESERIALIZE_VAL(OuterCutoff);

            return true;
        }
    };
}
