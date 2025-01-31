#pragma once
#include "Nuake/Core/Core.h"
#include "Nuake/Core/Logger.h"
#include "Nuake/Resource/UUID.h"

#include "Nuake/Rendering/Vertex.h"
#include "Nuake/Rendering/Vulkan/VulkanAllocatedBuffer.h"
#include "Nuake/Rendering/Vulkan/VulkanImage/VulkanImage.h"
#include "Nuake/Rendering/Vulkan/VkMesh.h"
#include "Nuake/Rendering/Textures/TextureManager.h"
#include "Nuake/Rendering/Vulkan/VulkanAllocatedBuffer.h"
#include "Nuake/Rendering/Vulkan/DescriptorLayoutBuilder.h"
#include "Nuake/Rendering/Vulkan/VulkanRenderer.h"

#include <volk/volk.h>

#include <stack>
#include <array>
#include <map>

namespace Nuake
{
	struct ModelData
	{
		std::array<Matrix4, 3000> Data;
	};

	// This is what is present on the shader as a structured buffer
	struct MaterialBufferStruct
	{
		int HasAlbedo;
		Vector3 AlbedoColor;
		int HasNormal;
		int HasMetalness;
		int HasRoughness;
		int HasAO;
		float MetalnessValue;
		float RoughnessValue;
		float AoValue;
		uint32_t AlbedoTextureId;
		uint32_t NormalTextureId;
		uint32_t MetalnessTextureId;
		uint32_t RoughnessTextureId;
		uint32_t AoTextureId;
	};

	// This is the *whole* buffer
	struct MaterialData
	{
		std::array<MaterialBufferStruct, 1000> Data;
	};

	struct LightData
	{
		Vector3 Position;
		int Type;
		Vector4 Color;
		Vector3 Direction;
		float OuterConeAngle;
		float InnerConeAngle;
		int CastShadow;
		int ShadowMapTextureId[4];
		int TransformId[4];
		float pad[2];
	};

	struct LightDataContainer
	{
		std::array<LightData, 100> Data;
	};

	struct CameraView
	{
		Matrix4 View;
		Matrix4 Projection;
		Matrix4 ViewProjection;
		Matrix4 InverseView;
		Matrix4 InverseProjection;
		Vector3 Position;
		float Near;
		float Far;
		float pad;
		float pad2;
		float pad3;
		//char padding[64];            // 124 bytes to reach 128 bytes
	};

	constexpr uint32_t MAX_MODEL_MATRIX = 3000;
	constexpr uint32_t MAX_MATERIAL = 1000;
	constexpr uint32_t MAX_TEXTURES = 500;
	constexpr uint32_t MAX_CAMERAS = 100;
	constexpr uint32_t MAX_LIGHTS = 100;

	class GPUResources
	{
	private:
		bool isDirty = false;
		std::map<UUID, Ref<AllocatedBuffer>> Buffers;
		std::map<UUID, Ref<VkMesh>> Meshes;
		std::map<UUID, Ref<VulkanImage>> Images;
		std::map<UUID, Ref<VulkanImage>> Light;
		std::vector<CameraView> Cameras;

		std::array<CameraView, MAX_CAMERAS> CamerasData;

		// Bindless buffer layouts
		VkDescriptorSetLayout ModelDescriptorLayout;
		VkDescriptorSetLayout TriangleBufferDescriptorLayout;
		VkDescriptorSetLayout SamplerDescriptorLayout;
		VkDescriptorSetLayout MaterialDescriptorLayout;
		VkDescriptorSetLayout TexturesDescriptorLayout;
		VkDescriptorSetLayout LightsDescriptorLayout;
		VkDescriptorSetLayout CamerasDescriptorLayout;

		std::map<UUID, uint32_t> BindlessTextureMapping;
		std::map<UUID, uint32_t> CameraMapping;

		Ref<AllocatedBuffer> CamerasBuffer;
		Ref<AllocatedBuffer> ModelBuffer;
		Ref<AllocatedBuffer> MaterialBuffer;
		Ref<AllocatedBuffer> LightBuffer;

		// Samplers
		VkSampler SamplerLinear;
		VkSampler SamplerNearest;

		std::stack<std::function<void()>> DeletionQueue;

	public:
		ModelData ModelTransforms;
		MaterialData MaterialDataContainer;

		LightDataContainer LightDataContainerArray;
		uint32_t LightCount = 0;
	public:
		std::map<UUID, uint32_t> ModelMatrixMapping;	// Holds mapping between model entity and transform index
		std::map<UUID, uint32_t> MeshMaterialMapping;	// Holds mapping between mesh and material index 

		VkDescriptorSet ModelDescriptor;
		VkDescriptorSet TriangleBufferDescriptor;
		VkDescriptorSet SamplerDescriptor;
		VkDescriptorSet MaterialDescriptor;
		VkDescriptorSet TexturesDescriptor;
		VkDescriptorSet LightsDescriptor;
		VkDescriptorSet CamerasDescriptor;

		static GPUResources& Get()
		{
			static GPUResources instance;
			return instance;
		};

		GPUResources();
		~GPUResources() = default;

	public:
		void Init();

		Ref<AllocatedBuffer> CreateBuffer(size_t size, BufferUsage flags, MemoryUsage usage, const std::string& name = "");
		bool AddBuffer(const Ref<AllocatedBuffer>& buffer);
		Ref<AllocatedBuffer> GetBuffer(const UUID& id);
		std::vector<Ref<AllocatedBuffer>> GetAllBuffers();

		Ref<VkMesh> CreateMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
		bool AddMesh(const Ref<VkMesh>& mesh);
		Ref<VkMesh> GetMesh(const UUID& id);

		bool AddTexture(Ref<VulkanImage> image);
        Ref<VulkanImage> GetTexture(const UUID& id);
		std::vector<Ref<VulkanImage>> GetAllTextures();

		void AddCamera(const UUID& id, const CameraView& camera);
		CameraView GetCamera(const UUID& id);
		std::vector<CameraView> GetAllCameras();
		void ClearCameras();

		std::vector<VkDescriptorSetLayout> GetBindlessLayout();
		std::vector<VkDescriptorSet> GetBindlessDescriptorSets();
		uint32_t GetBindlessTextureID(const UUID& id);
		uint32_t GetBindlessCameraID(const UUID& id);
		uint32_t GetBindlessTransformID(const UUID& id);
		uint32_t GetBindlessMaterialID(const UUID& id);

		void RecreateBindlessTextures();
		void RecreateBindlessCameras();
		void UpdateBuffers();

		void QueueDeletion(std::function<void()> func);
		void CleanUp();
	private:
		void CreateBindlessLayout();
	};
}