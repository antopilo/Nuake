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
	class VkSceneRenderer
	{
	public:
		RenderContext Context;

		std::map<std::string, Ref<VulkanShader>> Shaders;

		Ref<AllocatedBuffer> CameraBuffer;

		VkDescriptorSet TriangleBufferDescriptors;
		VkDescriptorSetLayout TriangleBufferDescriptorLayout;

		VkDescriptorSet CameraBufferDescriptors;
		VkDescriptorSetLayout CameraBufferDescriptorLayout;

		Ref<AllocatedBuffer> MaterialBuffer;
		VkDescriptorSet  MaterialBufferDescriptor;
		VkDescriptorSetLayout  MaterialBufferDescriptorLayout;

		Ref<AllocatedBuffer> TextureBuffer;
		VkDescriptorSet  TextureBufferDescriptor;
		VkDescriptorSetLayout  TextureBufferDescriptorLayout;

		Ref<AllocatedBuffer> LightBuffer;
		VkDescriptorSet LightBufferDescriptor;
		VkDescriptorSetLayout LightBufferDescriptorLayout;

		VkDescriptorSet SamplerDescriptor;
		VkDescriptorSetLayout SamplerDescriptorLayout;

		VkDescriptorSet ImageDescriptor;
		VkDescriptorSetLayout ImageDescriptorLayout;

		// New pipeline stuff
		//RenderPipeline GBufferPipeline;
		//RenderPipeline ShadowPipeline;

		Ref<SceneRenderPipeline> sceneRenderPipeline;
	public:
		UUID CurrentCamera;
		VkSceneRenderer();
		~VkSceneRenderer();

		void Init();

		void SetGBufferSize(const Vector2& size);

		void BeginScene(RenderContext inContext);
		void EndScene();

		//RenderPipeline& GetRenderPipeline() { return ShadowPipeline; }
	private:
		void LoadShaders();
		void CreateDescriptors();
		void CreatePipelines();

		void BuildMatrixBuffer();
		void UpdateTransformBuffer();
	};
}