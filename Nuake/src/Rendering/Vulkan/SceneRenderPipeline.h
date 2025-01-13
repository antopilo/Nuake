#pragma once
#include "src/Core/Core.h"
#include "src/Resource/UUID.h"

#include "src/Rendering/Vulkan/Pipeline/RenderPipeline.h"
#include "src/Rendering/Vulkan/VkResources.h"

namespace Nuake
{
	class ShadowRenderPipeline
	{
	private:
		UUID CurrentLightID;
		Ref<VulkanImage> DepthTexture;

		static RenderPipeline ShadowPipeline;
		
	public:
		ShadowRenderPipeline() = default;
		~ShadowRenderPipeline() = default;

		void Render();

		Ref<VulkanImage> GetOutput() { return DepthTexture; }
	};

	struct GBufferConstant
	{
		int Index;
		int MaterialIndex;
		int CameraID;
		char padding[120];            // 124 bytes to reach 128 bytes
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

		GBufferConstant gbufferConstant;
		ShadingConstant shadingConstant;

		// Holds all the texture attachments
		static RenderPipeline GBufferPipeline;

	public:
		SceneRenderPipeline();
		~SceneRenderPipeline() = default;

		void SetCamera(UUID camera);
		void Render(PassRenderContext& ctx);
		Ref<VulkanImage> GetOutput() { return ShadingOutput; }

	private:
		Ref<VulkanImage> ResizeImage(Ref<VulkanImage> image, const Vector2& size);
	};
}
