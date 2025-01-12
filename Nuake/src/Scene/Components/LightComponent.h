#pragma once

#include "Component.h"

#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float2.hpp>
#include "TransformComponent.h"
#include "../Rendering/Camera.h"
#include "src/Rendering/Buffers/Framebuffer.h"
#include "VisibilityComponent.h"
#include "../Resource/Serializable.h"
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

        Ref<FrameBuffer> m_Framebuffers[CSM_AMOUNT];
        Matrix4 mViewProjections[CSM_AMOUNT];
        std::vector<LightView> m_LightViews;
        float mCascadeSplitDepth[CSM_AMOUNT];
        float mCascadeSplits[CSM_AMOUNT];

    public:

        LightComponent();
        ~LightComponent() = default;

        UUID LightMapID;
        void SetCastShadows(bool toggle);

        Matrix4 GetProjection();
        Vector3 GetDirection();

        void CalculateViewProjection(glm::mat4& view, const glm::mat4& projection)
        {
            Matrix4 normalProj = projection;

            // Convert to normal Z
            //normalProj[2][2] = -normalProj[2][2];  // Restore the sign
            //normalProj[2][3] = -normalProj[2][3];  // Restore the far depth term sign

            //normalProj *= -1.0f;
            glm::mat4 viewProjection = normalProj * view;
            glm::mat4 inverseViewProjection = glm::inverse(viewProjection);

            // TODO: Automate this
            const float nearClip = 0.01f;
            const float farClip = 200.0f;
            const float clipRange = farClip - nearClip;

            const float mCascadeNearPlaneOffset = -100.0f;
            const float mCascadeFarPlaneOffset = 0.0;

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

            mCascadeSplits[0] = 0.01f;
            //mCascadeSplits[1] = 0.45f;
            //mCascadeSplits[2] = 1.0f;

            float lastSplitDist = 0.0f;
            // Calculate Orthographic Projection matrix for each cascade
            for (int cascade = 0; cascade < CSM_AMOUNT; cascade++)
            {
                float splitDist = mCascadeSplits[cascade];
                glm::vec4 frustumCorners[8] =
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
                    glm::vec4 invCorner = inverseViewProjection * frustumCorners[i];
                    frustumCorners[i] = invCorner / invCorner.w;
                }
                for (int i = 0; i < CSM_AMOUNT; i++)
                {
                    glm::vec4 dist = frustumCorners[i + 4] - frustumCorners[i];
                    frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
                    frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
                }

                // Get frustum center
                glm::vec3 frustumCenter = glm::vec3(0.0f);
                for (int i = 0; i < 8; i++)
                    frustumCenter += glm::vec3(frustumCorners[i]);
                frustumCenter /= 8.0f;

                // Get the minimum and maximum extents
                float radius = 0.0f;
                for (int i = 0; i < 8; i++)
                {
                    float distance = glm::length(glm::vec3(frustumCorners[i]) - frustumCenter);
                    radius = glm::max(radius, distance);
                }
                radius = std::ceil(radius * 16.0f) / 16.0f;
                glm::vec3 maxExtents = glm::vec3(radius);
                glm::vec3 minExtents = -maxExtents;

                // Calculate the view and projection matrix
                glm::vec3 lightDir = -this->Direction;
                lightDir.y *= -1.0f;
                lightDir.x *= -1.0f;
                lightDir.z *= -1.0f;
                glm::mat4 lightViewMatrix = glm::lookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter, glm::vec3(0.0f, 1.0, 0.0f));
                glm::mat4 lightProjectionMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f + mCascadeNearPlaneOffset, maxExtents.z - minExtents.z + mCascadeFarPlaneOffset);
                 
                //lightDir.y *= -1.0f;
                //
                //glm::mat4 lightViewMatrix = glm::lookAt(
                //    frustumCenter + lightDir * -minExtents.z,
                //    frustumCenter,
                //    glm::vec3(0.0f, 1.0f, 0.0f)
                //);
                //glm::mat4 lightProjectionMatrix = glm::ortho(
                //    minExtents.x, maxExtents.x,
                //    minExtents.y, maxExtents.y, // Y-flip for Vulkan
                //    0.0f + mCascadeNearPlaneOffset,
                //    maxExtents.z - minExtents.z + mCascadeFarPlaneOffset
                //);
				//lightProjectionMatrix = glm::ortho(-25.0f, 25.0f, -25.0f, 25.0f, 100.0f, -100.0f);
                // Offset to texel space to avoid shimmering ->(https://stackoverflow.com/questions/33499053/cascaded-shadow-map-shimmering)
                glm::mat4 shadowMatrix = lightProjectionMatrix * lightViewMatrix;
               //const float ShadowMapResolution = 4096;
               //glm::vec4 shadowOrigin = (shadowMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)) * ShadowMapResolution / 2.0f;
               //glm::vec4 roundedOrigin = glm::round(shadowOrigin);
               //glm::vec4 roundOffset = roundedOrigin - shadowOrigin;
               //roundOffset = roundOffset * 2.0f / ShadowMapResolution;
               //roundOffset.z = 0.0f;
               //roundOffset.w = 0.0f;
               //lightProjectionMatrix[3] += roundOffset;
                float near_plane = 0.01f, far_plane = 100.0f;
                //glm::mat4 lightProjection = glm::ortho(-25.0f, 25.0f, -25.0f, 25.0f, 25.0f, -25.0f);

                //lightProjectionMatrix[2][2] = -lightProjectionMatrix[2][2];  // Flip the sign
                //lightProjectionMatrix[2][3] = -lightProjectionMatrix[2][3];  // Flip the sign of the far depth term

                m_LightViews[cascade].View = lightViewMatrix;
                m_LightViews[cascade].Proj = lightProjectionMatrix;

                // Store SplitDistance and ViewProjection-Matrix
                mCascadeSplitDepth[cascade] = (nearClip + splitDist * clipRange) * 1.0f;
                mViewProjections[cascade] = shadowMatrix;
                lastSplitDist = mCascadeSplits[cascade];



                // -----------------------Debug only-----------------------
                // RendererDebug::BeginScene(viewProjection);
                // RendererDebug::SubmitCameraFrustum(frustumCorners, glm::mat4(1.0f), GetColor(cascade));   // Draws the divided camera frustums
                // RendererDebug::SubmitLine(glm::vec3(0.0f, 0.0f, 0.0f), frustumCenter, GetColor(cascade)); // Draws the center of the frustum (A line pointing from origin to the center)
                // RendererDebug::EndScene();
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
                Type = (LightType)j["Type"];
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
