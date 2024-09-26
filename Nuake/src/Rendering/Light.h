#pragma once
#include "src/Core/Maths.h"
#include "src/Rendering/Buffers/Framebuffer.h"

namespace Nuake {

	enum LightType
	{
		Point, Directional, Spot, Area
	};

	class Light
	{
	public:
		LightType Type = Point;
		bool Enabled;
		Color LightColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
		float Intensity = 1.0f;
		bool CastShadows;

	};

	class PointLight : public Light
	{
	public:
		
	private:
	};

	class DirectionalLight : public Light
	{
	public:
		DirectionalLight();
		~DirectionalLight();

		Vector3 Direction;

		void CSMViewCalculation(const Matrix4& camView, const Matrix4& camProjection);

		inline float GetCascadeSplitDepth(const int i) { return m_CascadeSplitDepths[i]; }
		inline float GetCascadeSplit(const int i) { return m_CascadeSplits[i]; }
		inline Matrix4 GetCascadeViewProjection(const int i) { return m_CascadeViewProjections[i]; }
	private:
		static const int CSM_SPLIT_AMOUNT = 4;
		const float CSM_NEAR_CLIP = 0.001f;
		const float CSM_FAR_CLIP = 400.0f;
		const float CSM_CLIP_RANGE = CSM_FAR_CLIP - CSM_NEAR_CLIP;

		const float mCascadeNearPlaneOffset = 0.0;
		const float mCascadeFarPlaneOffset = 0.0;

		float m_CascadeSplits[CSM_SPLIT_AMOUNT];
		float m_CascadeSplitDepths[CSM_SPLIT_AMOUNT];
		Matrix4 m_CascadeViewProjections[CSM_SPLIT_AMOUNT];

		Ref<FrameBuffer> m_ShadowMaps[CSM_SPLIT_AMOUNT];
	};

	class SpotLight : public Light
	{
	public:
		Matrix4 Projection;
		Vector3 Direction;
		float InnerRadius;
		float OuterRadius;
	private:
	};

	class AreaLight : public Light
	{
	public:
		Matrix4 Projection;
		Vector3 Direction;
		float FOV;
		float Height;
		float Width;
	private:
	};
}