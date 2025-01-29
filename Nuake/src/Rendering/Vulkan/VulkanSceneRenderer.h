#pragma once
#include "src/Core/Core.h"
#include "src/Core/Maths.h"
#include "src/Rendering/Vulkan/RenderContext.h"
#include "src/Rendering/Vulkan/VulkanShader.h"
#include "src/Rendering/Vulkan/VulkanAllocatedBuffer.h"
#include "src/Rendering/Vulkan/VulkanImage/VulkanImage.h"
#include "src/Rendering/Vulkan/Pipeline/RenderPipeline.h"

#include <volk/volk.h>

#include <vector>
#include <map>
#include <string>

namespace Nuake
{
	class SceneRenderPipeline;
	class ShadowRenderPipeline;
	class Scene;

	class VkMesh;

	class VkSceneRenderer
	{
	public:
		static Ref<VkMesh> QuadMesh;

	public:
		RenderContext Context;
		Ref<SceneRenderPipeline> sceneRenderPipeline;
		Ref<ShadowRenderPipeline> shadowRenderPipeline;

	public:
		UUID CurrentCamera;

		VkSceneRenderer() = default;
		~VkSceneRenderer() = default;

		void Init();
		void SetGBufferSize(const Vector2& size);

		void PrepareScenes(const std::vector<Ref<Scene>>& scenes, RenderContext inContext);
		void DrawSceneView(RenderContext inContext);
		void BeginScene(RenderContext inContext);
		void EndScene();

	private:
		void LoadShaders();
	};
}