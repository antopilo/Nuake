#pragma once
#include "src/Core/Core.h"
#include "src/Core/Maths.h"
#include "src/Rendering/Vulkan/RenderContext.h"
#include "src/Rendering/Vulkan/VulkanShader.h"
#include "src/Rendering/Vulkan/VulkanAllocatedBuffer.h"

#include <volk/volk.h>

#include <array>
#include <map>
#include <string>


namespace Nuake
{
	struct CameraData;

	struct ModelPushConstant
	{
		int Index;
		char padding[124];            // 124 bytes to reach 128 bytes
	};

	struct ModelData
	{
		std::array<Matrix4, 3000> Data;
	};

	class VkSceneRenderer
	{
	private:
		RenderContext Context;

		std::map<std::string, Ref<VulkanShader>> Shaders;
		VkPipeline BasicPipeline;
		VkPipelineLayout BasicPipelineLayout;

		Ref<AllocatedBuffer> CameraBuffer;

		VkDescriptorSet TriangleBufferDescriptors;
		VkDescriptorSetLayout TriangleBufferDescriptorLayout;

		VkDescriptorSet CameraBufferDescriptors;
		VkDescriptorSetLayout CameraBufferDescriptorLayout;

		Ref<AllocatedBuffer> ModelBuffer;
		VkDescriptorSet ModelBufferDescriptor;
		VkDescriptorSetLayout ModelBufferDescriptorLayout;

		ModelData ModelTransforms;
		std::map<UUID, uint32_t> ModelMatrixMapping;

		VkSampler SamplerLinear;
		VkSampler SamplerNearest;
		VkDescriptorSet SamplerDescriptor;
		VkDescriptorSetLayout SamplerDescriptorLayout;

		VkDescriptorSet ImageDescriptor;
		VkDescriptorSetLayout ImageDescriptorLayout;

		// GBuffer render targets
		//Ref<VulkanImage> GBufferAlbedo;
		//Ref<VulkanImage> GBufferDepthImage;

	public:
		VkSceneRenderer();
		~VkSceneRenderer();

		void Init();

		void UpdateCameraData(const CameraData& data);

		void BeginScene(RenderContext inContext);
		void DrawScene();
		void EndScene();

	private:
		void LoadShaders();
		void CreateBuffers();
		void CreateBasicPipeline();
		void CreateSamplers();
		void CreateDescriptors();

		void BuildMatrixBuffer();
		void UpdateTransformBuffer();
	};
}