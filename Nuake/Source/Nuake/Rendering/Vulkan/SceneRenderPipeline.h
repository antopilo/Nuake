#pragma once
#include "Nuake/Core/Core.h"
#include "Nuake/Resource/UUID.h"

#include "Nuake/Rendering/Vulkan/Constant/DebugConstant.h"
#include "Nuake/Rendering/Vulkan/Pipeline/RenderPipeline.h"
#include "Nuake/Rendering/Vulkan/VkResources.h"

#include "Nuake/Core/MulticastDelegate.h"
#include "Constant/LineConstant.h"

#include <functional>

namespace Nuake
{
	struct GBufferConstant
	{
		int Index;
		int MaterialIndex;
		int CameraID;
		float EntityID;
	};

	class ShadowRenderPipeline
	{
	private:
		UUID CurrentLightID;

		static RenderPipeline ShadowPipeline;
		GBufferConstant gbufferConstant;
	public:
		ShadowRenderPipeline();
		~ShadowRenderPipeline() = default;

		void Render(PassRenderContext& ctx, Ref<VulkanImage> output);
	};

	struct SSAOConstant
	{
		int noiseTextureID;
		int normalTextureID;
		int depthTextureID;
		int camViewID; 
		float radius;
		float bias;
		Vector2 noiseScale;
		float power;
	};

	struct SkyConstant
	{
		int cameraID;
		float surfaceRadius;
		float atmosphereRadius;
		Vector3 RayleighScattering;
		Vector3 MieScattering;
		float SunIntensity;
		Vector3 CenterPoint;
		Vector3 SunDirection;
		Vector3 CamDirection;
		Vector3 CamUp;
		Vector3 CamRight;
		float Exposure;
	};

	struct BlurConstant
	{
		int sourceTextureID;
		Vector2 sourceSize;
	};

	struct ShadingConstant
	{
		int AlbedoTextureID;
		int DepthTextureID;
		int NormalTextureID;
		int MaterialTextureID;
		int LightOffset;
		int LightCount;
		int CameraID;
		float AmbientTerm;
		float CascadeSplits[4];
		int SSAOTextureID;
		int EntityTextureID;
	};

	struct TonemapConstant
	{
		float Exposure;
		float Gamma;
		int SourceTextureID;
	};

	struct VolumetricConstant
	{
		int DepthTextureID;
		int StepCount;
		float FogAmount;
		float Exponant;
		int CamViewID;
		int LightCount;
		float Ambient;
		float Time;
		float NoiseSpeed;
		float NoiseScale;
		float NoiseStrength;
		float CSMSplits[4];
	};

	struct CopyConstant
	{
		int SourceTextureID;
		int Source2TextureID;
		int Mode;
	};

	struct OutlineConstant
	{
		Vector4 Color;
		float Thickness;
		int SourceTextureID;
		int EntityIDTextureID;
		int DepthTextureID;
		float SelectedEntityID;
	};

	struct BloomConstant
	{
		int Stage;
		int SourceTextureID;
		int HasLensDirt;
		int LensDirtTextureID;
		float LensDirtIntensity;
		float Threshold;
		float BlurAmount;
		int BlurDirection;
	};

	class DebugCmd;
	class DebugLineCmd;

	using MousePickingCb = std::function<void(int result)>;

	struct MousePickingRequest
	{
		Vector2 mousePosition;
		MousePickingCb callback;
	};

	// This class handles all the rendering of the scene
	class SceneRenderPipeline
	{
	private:
		UUID CurrentCameraID;

		static std::vector<Vector3> ssaoKernelSamples;
		static Ref<VulkanImage> ssaoNoiseTexture;

		std::vector<MousePickingRequest> mousePickingRequests;

		// Attachments GBuffer
		Ref<VulkanImage> GBufferAlbedo;
		Ref<VulkanImage> GBufferDepth;
		Ref<VulkanImage> GBufferNormal;
		Ref<VulkanImage> GBufferMaterial;
		Ref<VulkanImage> GBufferEntityID;

		// Attachments Shading
		Ref<VulkanImage> ShadingOutput;
		Ref<VulkanImage> SSAOOutput;
		Ref<VulkanImage> SSAOBlurOutput;

		Ref<VulkanImage> LineOutput;
		Ref<VulkanImage> LineCombineOutput;

		Ref<VulkanImage> GizmoOutput;
		Ref<VulkanImage> GizmoCombineOutput;

		Ref<VulkanImage> TonemappedOutput;
		Ref<VulkanImage> VolumetricOutput;
		Ref<VulkanImage> VolumetricBlurOutput;
		Ref<VulkanImage> VolumetricCombineOutput;

		Ref<VulkanImage> OutlineOutput;

		// Bloom
		const int BloomIteration = 4;
		const float DownsampleScale = 0.4f;
		Ref<VulkanImage> BloomThreshold;
		std::vector<Ref<VulkanImage>> BloomDownSample;
		std::vector<Ref<VulkanImage>> BloomUpSample;
		std::vector<Ref<VulkanImage>> BloomHBlur;
		std::vector<Ref<VulkanImage>> BloomVBlur;
		Ref<VulkanImage> BloomOutput;

		// Push constant
		GBufferConstant gbufferConstant;
		ShadingConstant shadingConstant;
		SSAOConstant ssaoConstant;
		BlurConstant blurConstant;
		SkyConstant skyConstant;
		TonemapConstant tonemapConstant;
		DebugConstant debugConstant;
		LineConstant lineConstant;
		CopyConstant copyConstant;
		OutlineConstant outlineConstant;
		BloomConstant bloomConstant;
		VolumetricConstant volumetricConstant;


		RenderPipeline GBufferPipeline;

		// Delegates
		MulticastDelegate<DebugCmd&> DebugDrawDelegate;
		MulticastDelegate<DebugLineCmd&> DebugLineDrawDelegate;


	public:
		SceneRenderPipeline();
		~SceneRenderPipeline();

		void SetCamera(UUID camera);
		void Render(PassRenderContext& ctx); 
		Ref<VulkanImage> GetOutput() { return LineCombineOutput; }

		MulticastDelegate<DebugCmd&>& OnDebugDraw() { return DebugDrawDelegate; }
		MulticastDelegate<DebugLineCmd&>& OnLineDraw() { return DebugLineDrawDelegate; }

		void RecreatePipeline();

		void MousePick(const Vector2& coord, MousePickingCb mousePickingCb);
	private:
		Ref<VulkanImage> ResizeImage(PassRenderContext& ctx, Ref<VulkanImage> image, const Vector2& size);
	};
}
 