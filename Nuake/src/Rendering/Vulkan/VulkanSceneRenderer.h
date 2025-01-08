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
	struct CameraData;

	struct ModelPushConstant
	{
		int Index;
		int MaterialIndex;
		char padding[120];            // 124 bytes to reach 128 bytes
	};

	struct ModelData
	{
		std::array<Matrix4, 3000> Data;
	};

	// This is what is present on the shader as a structured buffer
	struct MaterialBufferStruct
	{
		float hasAlbedo;
		Vector3 albedo;
		int hasNormal;
		int hasMetalness;
		int hasRoughness;
		int hasAO;
		float metalnessValue;
		float roughnessValue;
		float aoValue;
		float pad;
	};

	// This is the *whole* buffer
	struct MaterialData
	{
		std::array<MaterialBufferStruct, 1000> Data;
	};

	class VkSceneRenderer
	{
	private:
		RenderContext Context;

		std::map<std::string, Ref<VulkanShader>> Shaders;

		Ref<AllocatedBuffer> CameraBuffer;

		VkDescriptorSet TriangleBufferDescriptors;
		VkDescriptorSetLayout TriangleBufferDescriptorLayout;

		VkDescriptorSet CameraBufferDescriptors;
		VkDescriptorSetLayout CameraBufferDescriptorLayout;

		Ref<AllocatedBuffer> ModelBuffer;
		VkDescriptorSet ModelBufferDescriptor;
		VkDescriptorSetLayout ModelBufferDescriptorLayout;

		Ref<AllocatedBuffer> MaterialBuffer;
		VkDescriptorSet  MaterialBufferDescriptor;
		VkDescriptorSetLayout  MaterialBufferDescriptorLayout;

		ModelData ModelTransforms;
		MaterialData MaterialDataContainer;

		std::map<UUID, uint32_t> ModelMatrixMapping;	// Holds mapping between model entity and transform index
		std::map<UUID, uint32_t> MeshMaterialMapping;	// Holds mapping between mesh and material index 

		VkSampler SamplerLinear;
		VkSampler SamplerNearest;
		VkDescriptorSet SamplerDescriptor;
		VkDescriptorSetLayout SamplerDescriptorLayout;

		VkDescriptorSet ImageDescriptor;
		VkDescriptorSetLayout ImageDescriptorLayout;

		// New pipeline stuff
		RenderPipeline GBufferPipeline;

	public:
		VkSceneRenderer();
		~VkSceneRenderer();

		void Init();

		void SetGBufferSize(const Vector2& size);
		void UpdateCameraData(const CameraData& data);

		void BeginScene(RenderContext inContext);
		void EndScene();

	private:
		void LoadShaders();
		void CreateBuffers();
		void CreateSamplers();
		void CreateDescriptors();
		void CreatePipelines();

		void BuildMatrixBuffer();
		void UpdateTransformBuffer();
	};
}