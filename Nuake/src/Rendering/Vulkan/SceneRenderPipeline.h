#pragma once
#include "src/Core/Core.h"
#include "src/Resource/UUID.h"

#include "src/Rendering/Vulkan/Pipeline/RenderPipeline.h"
#include "src/Rendering/Vulkan/VkResources.h"

#include "src/Core/MulticastDelegate.h"

namespace Nuake
{
	struct GBufferConstant
	{
		int Index;
		int MaterialIndex;
		int CameraID;
		char padding[120];            // 124 bytes to reach 128 bytes
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
	};

	struct TonemapConstant
	{
		float Exposure;
		float Gamma;
		int SourceTextureID;
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

	// This class handles all the rendering of the scene
	class SceneRenderPipeline
	{
	private:
		UUID CurrentCameraID;

		// Attachments GBuffer
		Ref<VulkanImage> GBufferAlbedo;
		Ref<VulkanImage> GBufferDepth;
		Ref<VulkanImage> GBufferNormal;
		Ref<VulkanImage> GBufferMaterial;

		// Attachments Shading
		Ref<VulkanImage> ShadingOutput;

		Ref<VulkanImage> TonemappedOutput;

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
		TonemapConstant tonemapConstant;
		BloomConstant bloomConstant;

		static RenderPipeline GBufferPipeline;

		// Delegates
		MulticastDelegate<> DebugDrawDelegate;

	public:
		SceneRenderPipeline();
		~SceneRenderPipeline() = default;

		void SetCamera(UUID camera);
		void Render(PassRenderContext& ctx);
		Ref<VulkanImage> GetOutput() { return TonemappedOutput; }

		MulticastDelegate<>& OnDebugDraw() { return DebugDrawDelegate; }

	private:
		Ref<VulkanImage> ResizeImage(Ref<VulkanImage> image, const Vector2& size);
	};
}
