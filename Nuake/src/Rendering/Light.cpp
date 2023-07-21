#include "Light.h"
#include <src/Vendors/glm/ext/matrix_clip_space.hpp>
#include "src/Rendering/RenderCommand.h"
#include <dependencies/GLEW/include/GL/glew.h>

namespace Nuake
{
    DirectionalLight::DirectionalLight()
    {
        for (int i = 0; i < CSM_SPLIT_AMOUNT; i++)
        {
            Ref<FrameBuffer> shadowmap = CreateRef<FrameBuffer>(false, glm::vec2(4096, 4096));

            Ref<Texture> texture = CreateRef<Texture>(glm::vec2(4096, 4096), GL_DEPTH_COMPONENT);
            texture->SetParameter(GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
            shadowmap->SetTexture(texture, GL_DEPTH_ATTACHMENT);
            m_ShadowMaps[i] = shadowmap;
        }
    }

    DirectionalLight::~DirectionalLight() 
    {
        for (int i = 0; i < CSM_SPLIT_AMOUNT; i++)
        {

        }
    }

    void DirectionalLight::CSMViewCalculation(const Matrix4& camView, const Matrix4& camProjection)
	{
        Matrix4 viewProjection = camProjection * camView;
        Matrix4 inverseViewProjection = glm::inverse(viewProjection);

        // Calculate the optimal cascade distances
        const float range = CSM_FAR_CLIP - CSM_NEAR_CLIP;
        const float ratio = CSM_FAR_CLIP / CSM_NEAR_CLIP;
        for (int i = 0; i < CSM_SPLIT_AMOUNT; i++)
        {
            const float p = (i + 1) / static_cast<float>(4);
            const float log = CSM_NEAR_CLIP * glm::pow(ratio, p);
            const float uniform = CSM_NEAR_CLIP + range * p;
            const float d = 0.91f * (log - uniform) + uniform;
            m_CascadeSplits[i] = (d - CSM_NEAR_CLIP) / CSM_CLIP_RANGE;
        }

        float lastSplitDist = 0.0f;
        // Calculate Orthographic Projection matrix for each cascade
        for (int cascade = 0; cascade < CSM_SPLIT_AMOUNT; cascade++)
        {
            float splitDist = m_CascadeSplits[cascade];
            Vector4 frustumCorners[8] =
            {
                //Near face
                {  1.0f,  1.0f, -1.0f, 1.0f },
                { -1.0f,  1.0f, -1.0f, 1.0f },
                {  1.0f, -1.0f, -1.0f, 1.0f },
                { -1.0f, -1.0f, -1.0f, 1.0f },

                //Far face
                {  1.0f,  1.0f, 1.0f, 1.0f },
                { -1.0f,  1.0f, 1.0f, 1.0f },
                {  1.0f, -1.0f, 1.0f, 1.0f },
                { -1.0f, -1.0f, 1.0f, 1.0f },
            };

            // Project frustum corners into world space from clip space
            for (int i = 0; i < 8; i++)
            {
                Vector4 invCorner = inverseViewProjection * frustumCorners[i];
                frustumCorners[i] = invCorner / invCorner.w;
            }

            for (int i = 0; i < CSM_SPLIT_AMOUNT; i++)
            {
                Vector4 dist = frustumCorners[i + 4] - frustumCorners[i];
                frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
                frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
            }

            // Get frustum center
            Vector3 frustumCenter = Vector3(0.0f);
            for (int i = 0; i < 8; i++)
                frustumCenter += Vector3(frustumCorners[i]);
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
            Vector3 lightDir = -this->Direction;
            Matrix4 lightViewMatrix = glm::lookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter, Vector3(0.0f, 0.0f, 1.0f));
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
            lightProjectionMatrix[0] += roundOffset;
            lightProjectionMatrix[1] += roundOffset;
            lightProjectionMatrix[2] += roundOffset;
            lightProjectionMatrix[3] += roundOffset;

            // Store SplitDistance and ViewProjection-Matrix
            m_CascadeSplitDepths[cascade] = (CSM_NEAR_CLIP + splitDist * CSM_FAR_CLIP) * 1.0f;
            m_CascadeViewProjections[cascade] = lightProjectionMatrix * lightViewMatrix;
            lastSplitDist = m_CascadeSplits[cascade];

            // -----------------------Debug only-----------------------
            // RendererDebug::BeginScene(viewProjection);
            // RendererDebug::SubmitCameraFrustum(frustumCorners, glm::mat4(1.0f), GetColor(cascade));   // Draws the divided camera frustums
            // RendererDebug::SubmitLine(glm::vec3(0.0f, 0.0f, 0.0f), frustumCenter, GetColor(cascade)); // Draws the center of the frustum (A line pointing from origin to the center)
            // RendererDebug::EndScene();
        }
	}
}