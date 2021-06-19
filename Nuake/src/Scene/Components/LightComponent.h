#pragma once
#include <glm\ext\vector_float3.hpp>
#include <glm\ext\vector_float2.hpp>
#include "TransformComponent.h"
#include "../Rendering/Camera.h"
#include "../Rendering/Framebuffer.h"
#include "BaseComponent.h"
#include "../Resource/Serializable.h"

enum LightType {
	Directional, Point, Spot
};

class LightComponent  {
	
public:
	glm::vec2 yes = glm::vec2(2, 2);
	
	LightType Type = Point;
	glm::vec3 Direction = glm::vec3(0, -1, 0);
	glm::vec3 Color;
	bool IsVolumetric = false;
	float Strength;
	bool SyncDirectionWithSky = false;
	Ref<FrameBuffer> m_Framebuffer;

	bool CastShadows = true;
	float Attenuation = 0.0f;
	float LinearAttenuation = 0.0f;
	float QuadraticAttenuation = 0.0f;

	LightComponent();

	glm::mat4 GetProjection();

	glm::mat4 GetLightTransform();
	void SetDirection(glm::vec3 dir);
	glm::vec3 GetDirection();

	void BeginDrawShadow();

	void EndDrawShadow();
	void DrawShadow();

	void Draw(TransformComponent transformComponent, Ref<Camera> cam);
	void DrawDeferred(TransformComponent transformComponent, Camera* cam);
	void DrawEditor();

	void SetType(LightType type);

    std::vector<int> mCascadeSplits;
    void CalculateViewProjection(glm::mat4& view, const glm::mat4& projection, const glm::vec3& normalizedDirection)
    {
        glm::mat4 viewProjection = projection * view;
        glm::mat4 inverseViewProjection = glm::inverse(viewProjection);

        // TODO: Automate this
        const float nearClip = 0.1f;
        const float farClip = 1000.0f;
        const float clipRange = farClip - nearClip;

        // Calculate the optimal cascade distances
        const float minZ = nearClip;
        const float maxZ = nearClip + clipRange;
        const float range = maxZ - minZ;
        const float ratio = maxZ / minZ;
        for (int i = 0; i < 4; i++)
        {
            const float p = (i + 1) / static_cast<float>(4);
            const float log = minZ * glm::pow(ratio, p);
            const float uniform = minZ + range * p;
            //const float d = 0.91f * (log - uniform) + uniform;
            //mCascadeSplits[i] = (d - nearClip) / clipRange;
        }

        //mCascadeSplits[0] = 0.2f;
        //mCascadeSplits[1] = 0.45f;
        //mCascadeSplits[2] = 1.0f;

        float lastSplitDist = 0.0f;
        // Calculate Orthographic Projection matrix for each cascade
        for (int cascade = 0; cascade < 4; cascade++)
        {
            float splitDist = mCascadeSplits[cascade];
            glm::vec4 frustumCorners[8] =
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
                glm::vec4 invCorner = inverseViewProjection * frustumCorners[i];
                frustumCorners[i] = invCorner / invCorner.w;
            }
            for (int i = 0; i < 4; i++)
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
            glm::vec3 lightDir = -normalizedDirection;
            glm::mat4 lightViewMatrix = glm::lookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter, glm::vec3(0.0f, 0.0f, 1.0f));
            //glm::mat4 lightProjectionMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f + mCascadeNearPlaneOffset, maxExtents.z - minExtents.z + mCascadeFarPlaneOffset);

            // Offset to texel space to avoid shimmering ->(https://stackoverflow.com/questions/33499053/cascaded-shadow-map-shimmering)
            //glm::mat4 shadowMatrix = lightProjectionMatrix * lightViewMatrix;
            const float ShadowMapResolution = 4096;
            //glm::vec4 shadowOrigin = (shadowMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)) * ShadowMapResolution / 2.0f;
            //glm::vec4 roundedOrigin = glm::round(shadowOrigin);
            //glm::vec4 roundOffset = roundedOrigin - shadowOrigin;
            //roundOffset = roundOffset * 2.0f / ShadowMapResolution;
            //roundOffset.z = 0.0f;
            //roundOffset.w = 0.0f;
            //lightProjectionMatrix[3] += roundOffset;

            // Store SplitDistance and ViewProjection-Matrix
            //mCascadeSplitDepth[cascade] = (nearClip + splitDist * clipRange) * 1.0f;
            //mViewProjections[cascade] = lightProjectionMatrix * lightViewMatrix;
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
		END_SERIALIZE();
	}

	bool Deserialize(std::string str)
	{
		BEGIN_DESERIALIZE();
		if (j.contains("Type"))
			Type = (LightType)j["Type"];
		if (j.contains("IsVolumetric"))
			IsVolumetric = j["IsVolumetric"];
		if (j.contains("Strength"))
			Strength = j["Strength"];
		if (j.contains("SyncDirectionWithSky"))
			SyncDirectionWithSky = j["SyncDirectionWithSky"];
		if (j.contains("CastShadows"))
			SyncDirectionWithSky = j["CastShadows"];
		if (j.contains("Direction"))
		{
			float x = j["Direction"]["x"];
			float y = j["Direction"]["y"];
			float z = j["Direction"]["z"];
			this->Direction = Vector3(x, y, z);
		}

		return true;
	}
};