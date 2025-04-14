#pragma once
#include "Nuake/Core/Core.h"
#include "Nuake/Core/Maths.h"
#include "Nuake/Rendering/Vulkan/RenderContext.h"
#include "Nuake/Rendering/Vulkan/VulkanShader.h"
#include "Nuake/Rendering/Vulkan/VulkanAllocatedBuffer.h"
#include "Nuake/Rendering/Vulkan/VulkanImage/VulkanImage.h"
#include "Nuake/Rendering/Vulkan/Pipeline/RenderPipeline.h"

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
		static Ref<VkMesh> BoxMesh;
		static Ref<VkMesh> CapsuleMesh;
		static Ref<VkMesh> SphereMesh;
		static Ref<VkMesh> CylinderMesh;
		static Ref<VkMesh> ArrowMesh;
		static Ref<VkMesh> ConeMesh;

	public:
		//RenderContext Context;
		Ref<SceneRenderPipeline> sceneRenderPipeline;
		Ref<ShadowRenderPipeline> shadowRenderPipeline;

	public:
		VkSceneRenderer() = default;
		~VkSceneRenderer() = default;

		void Init();

		void PrepareScenes(const std::vector<Ref<Scene>>& scenes, RenderContext inContext);
		void DrawSceneView(RenderContext inContext);

		void RecreatePipelines();

	private:
		void LoadShaders();
	};
}