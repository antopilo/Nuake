#pragma once
#include "Nuake/Core/Core.h"
#include "Nuake/Resource/UUID.h"

#include "Nuake/Rendering/Vulkan/Constant/DebugConstant.h"
#include "Nuake/Rendering/Vulkan/Pipeline/RenderPipeline.h"
#include "Nuake/Rendering/Vulkan/VkResources.h"

#include "Nuake/Core/MulticastDelegate.h"
#include "Constant/LineConstant.h"

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
		int LightCount;
		int CameraID;
		float CascadeSplits[4];
		float AmbientTerm;
		int SSAOTextureID;
	};

	struct TonemapConstant
	{
		float Exposure;
		float Gamma;
		int SourceTextureID;
	};

	struct CopyConstant
	{
		int SourceTextureID;
		int Source2TextureID;
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

	// This class handles all the rendering of the scene
	class SceneRenderPipeline
	{
	private:
		UUID CurrentCameraID;

		static std::vector<Vector3> ssaoKernelSamples;
		static Ref<VulkanImage> ssaoNoiseTexture;

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
		TonemapConstant tonemapConstant;
		DebugConstant debugConstant;
		LineConstant lineConstant;
		CopyConstant copyConstant;
		OutlineConstant outlineConstant;
		BloomConstant bloomConstant;

		RenderPipeline GBufferPipeline;

		// Delegates
		MulticastDelegate<DebugCmd&> DebugDrawDelegate;
		MulticastDelegate<DebugLineCmd&> DebugLineDrawDelegate;

	public:
		SceneRenderPipeline();
		~SceneRenderPipeline() = default;

		void SetCamera(UUID camera);
		void Render(PassRenderContext& ctx); 
		Ref<VulkanImage> GetOutput() { return GizmoCombineOutput; }       

		MulticastDelegate<DebugCmd&>& OnDebugDraw() { return DebugDrawDelegate; }
		MulticastDelegate<DebugLineCmd&>& OnLineDraw() { return DebugLineDrawDelegate; }

		void RecreatePipeline();
	private:
		Ref<VulkanImage> ResizeImage(PassRenderContext& ctx, Ref<VulkanImage> image, const Vector2& size);
	};
}
 