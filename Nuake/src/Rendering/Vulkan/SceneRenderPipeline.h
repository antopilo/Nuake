#pragma once
#include "src/Core/Core.h"
#include "src/Resource/UUID.h"

#include "src/Rendering/Vulkan/Pipeline/RenderPipeline.h"

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

		// Holds all the texture attachments
		static RenderPipeline GBufferPipeline;


	public:
		SceneRenderPipeline(UUID camera);

		SceneRenderPipeline() = default;
		~SceneRenderPipeline() = default;

		void Render(PassRenderContext& ctx);
		Ref<VulkanImage> GetOutput() { return ShadingOutput; }

	private:
		Ref<VulkanImage> ResizeImage(Ref<VulkanImage> image, const Vector2& size);
	};
}
