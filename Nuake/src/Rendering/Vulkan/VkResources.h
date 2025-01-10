#pragma once
#include "src/Core/Core.h"
#include "src/Core/Logger.h"
#include "src/Resource/UUID.h"

#include "src/Rendering/Vertex.h"
#include "src/Rendering/Vulkan/VulkanAllocatedBuffer.h"
#include "src/Rendering/Vulkan/VulkanImage/VulkanImage.h"
#include "src/Rendering/Vulkan/VkMesh.h"
#include "src/Rendering/Textures/TextureManager.h"

#include "src/Rendering/Vulkan/DescriptorLayoutBuilder.h"
#include "src/Rendering/Vulkan/VulkanRenderer.h"

#include <volk/volk.h>

#include <map>

namespace Nuake
{
	struct LightResource
	{

	};

	class GPUResources
	{
	private:
		bool isDirty = false;
		std::map<UUID, Ref<AllocatedBuffer>> Buffers;
		std::map<UUID, Ref<VkMesh>> Meshes;
		std::map<UUID, Ref<VulkanImage>> Images;
		std::map<UUID, Ref<VulkanImage>> Light;

		// Bindless buffer layouts
		VkDescriptorSetLayout CameraDescriptorLayout;
		VkDescriptorSetLayout TriangleBufferDescriptorLayout;
		VkDescriptorSetLayout ModelBufferDescriptorLayout;
		VkDescriptorSetLayout ImageDescriptorLayout;
		VkDescriptorSetLayout SamplerDescriptorLayout;
		VkDescriptorSetLayout MaterialDescriptorLayout;
		VkDescriptorSetLayout TexturesDescriptorLayout;
		VkDescriptorSetLayout LightsDescriptorLayout;

		VkDescriptorSet CameraDescriptor;
		VkDescriptorSet ModelDescriptor;
		VkDescriptorSet SamplerDescriptor;
		VkDescriptorSet MaterialDescriptor;
		VkDescriptorSet LightsDescriptor;

		std::map<UUID, uint32_t> BindlessTextureMapping;

	public:
		VkDescriptorSet TextureDescriptor;
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

		std::vector<VkDescriptorSetLayout> GetBindlessLayout();

		uint32_t GetBindlessTextureID(const UUID& id);
		void RecreateBindlessTextures();

	private:
		void CreateBindlessLayout();
	};
}