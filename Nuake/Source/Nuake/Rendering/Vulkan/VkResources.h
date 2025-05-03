#pragma once
#include "Nuake/Core/Core.h"
#include "Nuake/Core/Logger.h"
#include "Nuake/Resource/UUID.h"

#include "Nuake/Rendering/Textures/TextureManager.h"
#include "Nuake/Rendering/Vertex.h"
#include "Nuake/Rendering/Vulkan/DescriptorLayoutBuilder.h"
#include "Nuake/Rendering/Vulkan/GPUManaged.h"
#include "Nuake/Rendering/Vulkan/VkMesh.h"
#include "Nuake/Rendering/Vulkan/VulkanAllocatedBuffer.h"
#include "Nuake/Rendering/Vulkan/VulkanImage/VulkanImage.h"
#include "Nuake/Rendering/Vulkan/VulkanRenderer.h"

#include "Nuake/Rendering/Vulkan/BindlessDescriptor.h"
#include "Nuake/Rendering/Vulkan/GPUData/GPUData.h"

#include <volk/volk.h>

#include <stack>
#include <array>
#include <map>

namespace Nuake
{
	// This is the *whole* buffer
	struct MaterialData
	{
		std::array<MaterialBufferStruct, 2000> Data;
	};

	struct LightDataContainer
	{
		std::array<LightData, 100> Data;
	};

	constexpr uint32_t MAX_MODEL_MATRIX = 3000;
	constexpr uint32_t MAX_MATERIAL = 2000;
	constexpr uint32_t MAX_TEXTURES = 3000;
	constexpr uint32_t MAX_CAMERAS = 1000;
	constexpr uint32_t MAX_LIGHTS = 100;

	class GPUResources
	{
		Ref<AllocatedBuffer> AllBuffer;
		struct FrameData
		{
			VkDescriptorSet TestDescriptorSet;
		};

	private:
		Scope<ResourceDescriptors> resourceDescriptors;

		FrameData frameData[FRAME_OVERLAP];

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

		VkDescriptorSetLayout SSAOKernelDescriptorLayout;

		std::map<UUID, uint32_t> BindlessTextureMapping;
		std::map<UUID, uint32_t> CameraMapping;

		Ref<AllocatedBuffer> CamerasBuffer;
		Ref<AllocatedBuffer> ModelBuffer;
		Ref<AllocatedBuffer> MaterialBuffer;
		Ref<AllocatedBuffer> LightBuffer;

		// Samplers
		VkSampler SamplerLinear;
		VkSampler SamplerNearest;

		std::vector<CleanUpStack> DeletionQueue;

	public:
		TransformData ModelTransforms;
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
		VkDescriptorSet SSAOKernelDescriptor;
		VkDescriptorSet TexturesDescriptor;
		VkDescriptorSet LightsDescriptor;
		VkDescriptorSet CamerasDescriptor;

		static GPUResources& Get()
		{
			static GPUResources instance;
			return instance;
		};

		GPUResources();
		~GPUResources();

	public:
		std::map<UUID, std::pair<uint32_t, uint32_t>> SceneLightOffets;
		void Init();

		Ref<AllocatedBuffer> CreateBuffer(size_t size, BufferUsage flags, MemoryUsage usage, const std::string& name = "");
		void CopyIntoBuffer(Ref<AllocatedBuffer> buffer, void* data, size_t size, VkDescriptorSet descSet);
		bool AddBuffer(const Ref<AllocatedBuffer>& buffer);
		Ref<AllocatedBuffer> GetBuffer(const UUID& id);
		std::vector<Ref<AllocatedBuffer>> GetAllBuffers();

		Ref<VkMesh> CreateMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
		bool AddMesh(const Ref<VkMesh>& mesh);
		Ref<VkMesh> GetMesh(const UUID& id);

		bool AddTexture(Ref<VulkanImage> image);
		void RemoveTexture(Ref<VulkanImage> image);
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

		void QueueDeletion(CleanUpStack func);
		void CleanUp(uint32_t frame);

		void Swap(uint32_t frame);
	private:
		void CreateBindlessLayout();
		CleanUpStack& GetFrameCleanUpStack(uint32_t frame);
	};
}