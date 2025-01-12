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
#include "SceneRenderPipeline.h"


namespace Nuake
{
	struct CameraData;

	struct ModelPushConstant
	{
		int Index;
		int MaterialIndex;
		int CameraID;
		char padding[120];            // 124 bytes to reach 128 bytes
	};

	struct ShadingPushConstant
	{
		int AlbedoTextureID;
		int DepthTextureID;
		int NormalTextureID;
		int MaterialTextureID;
		int LightCount;
		int CameraID;
		float CascadeSplits[4];
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
		int albedoTextureId;
		int normalTextureId;
		int metalnessTextureId;
		int roughnessTextureId;
		int aoTextureId;
	};

	// This is the *whole* buffer
	struct MaterialData
	{
		std::array<MaterialBufferStruct, 1000> Data;
	};

	struct LightData
	{
		Vector3 position;
		int type;
		Vector4 color;
		Vector3 direction;
		float outerConeAngle;
		float innerConeAngle;
		bool castShadow;
		int shadowMapTextureId[4];
		int transformId[4];
		float pad[2];
	};

	struct LightDataContainer
	{
		std::array<LightData, 100> Data;
	};



	struct CameraSceneRenderer
	{
		
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

		Ref<AllocatedBuffer> TextureBuffer;
		VkDescriptorSet  TextureBufferDescriptor;
		VkDescriptorSetLayout  TextureBufferDescriptorLayout;

		ModelData ModelTransforms;
		MaterialData MaterialDataContainer;

		LightDataContainer LightDataContainerArray;

		std::map<UUID, uint32_t> ModelMatrixMapping;	// Holds mapping between model entity and transform index
		std::map<UUID, uint32_t> MeshMaterialMapping;	// Holds mapping between mesh and material index 

		Ref<AllocatedBuffer> LightBuffer;
		VkDescriptorSet LightBufferDescriptor;
		VkDescriptorSetLayout LightBufferDescriptorLayout;

		VkSampler SamplerLinear;
		VkSampler SamplerNearest;
		VkDescriptorSet SamplerDescriptor;
		VkDescriptorSetLayout SamplerDescriptorLayout;

		VkDescriptorSet ImageDescriptor;
		VkDescriptorSetLayout ImageDescriptorLayout;

		// New pipeline stuff
		RenderPipeline GBufferPipeline;
		RenderPipeline ShadowPipeline;

		SceneRenderPipeline sceneRenderPipeline;
	public:
		UUID CurrentCamera;
		VkSceneRenderer();
		~VkSceneRenderer();

		void Init();

		void SetGBufferSize(const Vector2& size);
		void UpdateCameraData(const CameraData& data);

		void BeginScene(RenderContext inContext);
		void EndScene();

		RenderPipeline& GetRenderPipeline() { return ShadowPipeline; }
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