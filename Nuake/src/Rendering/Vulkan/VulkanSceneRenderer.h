#pragma once
#include "src/Core/Core.h"
#include "src/Core/Maths.h"
#include "src/Rendering/Vulkan/RenderContext.h"
#include "src/Rendering/Vulkan/VulkanShader.h"
#include "src/Rendering/Vulkan/VulkanAllocatedBuffer.h"
#include "src/Rendering/Vulkan/VulkanImage/VulkanImage.h"
#include "src/Rendering/Vulkan/Pipeline/RenderPipeline.h"

#include <volk/volk.h>

#include <array>
#include <map>
#include <string>

namespace Nuake
{
	class SceneRenderPipeline;
	class VkMesh;

	class VkSceneRenderer
	{
	public:
		static Ref<VkMesh> QuadMesh;

	public:
		RenderContext Context;
		Ref<SceneRenderPipeline> sceneRenderPipeline;

	public:
		UUID CurrentCamera;

		VkSceneRenderer() = default;
		~VkSceneRenderer() = default;

		void Init();
		void SetGBufferSize(const Vector2& size);

		void BeginScene(RenderContext inContext);
		void EndScene();

	private:
		void LoadShaders();
	};
}