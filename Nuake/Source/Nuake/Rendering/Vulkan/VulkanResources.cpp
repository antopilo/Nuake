#include "VkResources.h"

#include "VulkanAllocator.h"

#include "GPUManaged.h"

using namespace Nuake;

GPUResources::GPUResources()
{
	Init();

	for (int i = 0; i < FRAME_OVERLAP; i++)
	{
		DeletionQueue.push_back(CleanUpStack());
	}
}

GPUResources::~GPUResources()
{
	Images.clear();
	Buffers.clear();
	Meshes.clear();
	Light.clear();
	Cameras.clear();
}

void GPUResources::Init()
{
	CreateBindlessLayout();
	CamerasBuffer = CreateBuffer(sizeof(MaterialBufferStruct) * MAX_MATERIAL, BufferUsage::STORAGE_BUFFER | BufferUsage::TRANSFER_DST, MemoryUsage::GPU_ONLY, "CamerasBuffer");
	ModelBuffer = CreateBuffer(sizeof(Matrix4) * MAX_MODEL_MATRIX, BufferUsage::STORAGE_BUFFER | BufferUsage::TRANSFER_DST, MemoryUsage::GPU_ONLY, "TransformBuffer");
	MaterialBuffer = CreateBuffer(sizeof(MaterialBufferStruct) * MAX_MATERIAL, BufferUsage::STORAGE_BUFFER | BufferUsage::TRANSFER_DST, MemoryUsage::GPU_ONLY, "MaterialBuffer");
	LightBuffer = CreateBuffer(sizeof(LightData) * MAX_LIGHTS, BufferUsage::STORAGE_BUFFER | BufferUsage::TRANSFER_DST, MemoryUsage::GPU_ONLY, "LightBuffer");
}

Ref<AllocatedBuffer> GPUResources::CreateBuffer(size_t size, BufferUsage flags, MemoryUsage usage, const std::string& name)
{
	Ref<AllocatedBuffer> buffer = CreateRef<AllocatedBuffer>(name, size, flags, usage);
	Buffers[buffer->GetID()] = buffer;
	return buffer;
}

void GPUResources::CopyIntoBuffer(Ref<AllocatedBuffer> buffer, void* data, size_t size, VkDescriptorSet descSet)
{
	void* mappedData;
	vmaMapMemory(VulkanAllocator::Get().GetAllocator(), (buffer->GetAllocation()), &mappedData);
	memcpy(mappedData, &data, size);

	VkRenderer::Get().ImmediateSubmit([&](VkCommandBuffer cmd) 
	{
		VkBufferCopy copy{ 0 };
		copy.dstOffset = 0;
		copy.srcOffset = 0;
		copy.size = size;
	
		vkCmdCopyBuffer(cmd, buffer->GetBuffer(), buffer->GetBuffer(), 1, &copy);
	});

	vmaUnmapMemory(VulkanAllocator::Get().GetAllocator(), buffer->GetAllocation());

	// Update descriptor set for camera
	VkDescriptorBufferInfo transformBufferInfo{};
	transformBufferInfo.buffer = buffer->GetBuffer();
	transformBufferInfo.offset = 0;
	transformBufferInfo.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet bufferWriteModel = {};
	bufferWriteModel.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	bufferWriteModel.pNext = nullptr;
	bufferWriteModel.dstBinding = 0;
	bufferWriteModel.dstSet = descSet;
	bufferWriteModel.descriptorCount = 1;
	bufferWriteModel.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bufferWriteModel.pBufferInfo = &transformBufferInfo;
	vkUpdateDescriptorSets(VkRenderer::Get().GetDevice(), 1, &bufferWriteModel, 0, nullptr);
}

bool GPUResources::AddBuffer(const Ref<AllocatedBuffer>& buffer)
{
	const UUID id = buffer->GetID();
	if (Buffers.find(id) == Buffers.end())
	{
		Buffers[id] = buffer;
		return true;
	}

	Logger::Log("Buffer with ID already exists", "vulkan", CRITICAL);
	return false;
}

Ref<AllocatedBuffer> GPUResources::GetBuffer(const UUID& id)
{
	if (Buffers.find(id) != Buffers.end())
	{
		return Buffers[id];
	}

	Logger::Log("Buffer with ID does not exist", "vulkan", CRITICAL);
	return nullptr;
}

std::vector<Ref<AllocatedBuffer>> GPUResources::GetAllBuffers()
{
	std::vector<Ref<AllocatedBuffer>> allBuffers;
	allBuffers.reserve(Buffers.size());
	for (const auto& [id, buffer] : Buffers)
	{
		allBuffers.push_back(buffer);
	}
	return allBuffers;
}

Ref<VkMesh> GPUResources::CreateMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
	Ref<VkMesh> mesh = CreateRef<VkMesh>(vertices, indices);
	Meshes[mesh->GetID()] = mesh;
	return mesh;
}

bool GPUResources::AddMesh(const Ref<VkMesh>& mesh)
{
	const UUID id = mesh->GetID();
	if (Meshes.find(id) == Meshes.end())
	{
		Meshes[id] = mesh;
		return true;
	}
	Logger::Log("Mesh with ID already exists", "vulkan", CRITICAL);
	return false;
}

Ref<VkMesh> GPUResources::GetMesh(const UUID& id)
{
	if (Meshes.find(id) != Meshes.end())
	{
		return Meshes[id];
	}
	Logger::Log("Mesh with ID does not exist", "vulkan", CRITICAL);
	return nullptr;
}

bool GPUResources::AddTexture(Ref<VulkanImage> image)
{
	const UUID id = image->GetID();
	if (Images.find(id) == Images.end())
	{
		Images[id] = image;
		return true;
	}
	
	Logger::Log("Buffer with ID already exists", "vulkan", CRITICAL);
	return false;
}

void GPUResources::RemoveTexture(Ref<VulkanImage> image)
{
	const UUID id = image->GetID();
	if (Images.find(id) == Images.end())
	{
		return;
	}
	Images.erase(id);
}

Ref<VulkanImage> GPUResources::GetTexture(const UUID& id)
{
	if (Images.find(id) != Images.end())
	{
		return Images[id];
	}

	Logger::Log("Texture with ID does not exist", "vulkan", CRITICAL);
	return TextureManager::Get()->GetTexture2("missing_texture");
}

std::vector<Ref<VulkanImage>> GPUResources::GetAllTextures()
{
	std::vector<Ref<VulkanImage>> allImages;
	allImages.reserve(Images.size());
	for (const auto& [id, image] : Images)
	{
		allImages.push_back(image);
	}
	return allImages;
}

void GPUResources::AddCamera(const UUID& id, const CameraView& camera)
{
	Cameras.push_back(camera);
	CameraMapping[id] = Cameras.size() - 1;
}

CameraView GPUResources::GetCamera(const UUID& id)
{
	if (CameraMapping.find(id) != CameraMapping.end())
	{
		return Cameras[CameraMapping[id]];
	}

	// Logger::Log("Camera with ID does not exist", "vulkan", CRITICAL);
	return Cameras[0];
}

std::vector<CameraView> GPUResources::GetAllCameras()
{
	return Cameras;
}

void GPUResources::ClearCameras()
{
	Cameras.clear();
	CameraMapping.clear();
}

void GPUResources::CreateBindlessLayout()
{
	auto& vk = VkRenderer::Get();
	auto device = vk.GetDevice();

	// Matrices
	{
		DescriptorLayoutBuilder builder;
		builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		ModelDescriptorLayout = builder.Build(device, VK_SHADER_STAGE_ALL_GRAPHICS);
	}

	// Triangles
	{
		DescriptorLayoutBuilder builder;
		builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		TriangleBufferDescriptorLayout = builder.Build(device, VK_SHADER_STAGE_ALL_GRAPHICS);
	}

	// SSAO kernel
	{
		DescriptorLayoutBuilder builder;
		builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		SSAOKernelDescriptorLayout = builder.Build(device, VK_SHADER_STAGE_ALL_GRAPHICS);
	}

	// Samplers
	{
		DescriptorLayoutBuilder builder;
		builder.AddBinding(0, VK_DESCRIPTOR_TYPE_SAMPLER);
		SamplerDescriptorLayout = builder.Build(device, VK_SHADER_STAGE_ALL_GRAPHICS);
	}

	// Material
	{
		DescriptorLayoutBuilder builder;
		builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		MaterialDescriptorLayout = builder.Build(device, VK_SHADER_STAGE_ALL_GRAPHICS);
	}

	// Textures
	{
		DescriptorLayoutBuilder builder;
		builder.AddBinding(0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, MAX_TEXTURES);
		TexturesDescriptorLayout = builder.Build(device, VK_SHADER_STAGE_ALL_GRAPHICS);
	}

	// bindless lights
	{
		DescriptorLayoutBuilder builder;
		builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		LightsDescriptorLayout = builder.Build(device, VK_SHADER_STAGE_ALL_GRAPHICS);
	}

	// bindless cameras
	{
		DescriptorLayoutBuilder builder;
		builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		CamerasDescriptorLayout = builder.Build(device, VK_SHADER_STAGE_ALL_GRAPHICS);
	}

	auto allocator = vk.GetDescriptorAllocator();
	TexturesDescriptor = allocator.Allocate(device, TexturesDescriptorLayout);
	CamerasDescriptor = allocator.Allocate(device, CamerasDescriptorLayout);
	TriangleBufferDescriptor = allocator.Allocate(device, TriangleBufferDescriptorLayout);
	SamplerDescriptor = allocator.Allocate(device, SamplerDescriptorLayout);
	ModelDescriptor = allocator.Allocate(device, ModelDescriptorLayout);
	LightsDescriptor = allocator.Allocate(device, LightsDescriptorLayout);
	MaterialDescriptor = allocator.Allocate(device, MaterialDescriptorLayout);
	SSAOKernelDescriptor = allocator.Allocate(device, SSAOKernelDescriptorLayout);

	// Samplers
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(VkRenderer::Get().GetPhysicalDevice(), &properties);

	VkSamplerCreateInfo sampler = { .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
	sampler.magFilter = VK_FILTER_NEAREST;
	sampler.minFilter = VK_FILTER_NEAREST;
	sampler.anisotropyEnable = VK_TRUE;
	sampler.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	vkCreateSampler(device, &sampler, nullptr, &SamplerNearest);

	sampler.magFilter = VK_FILTER_LINEAR;
	sampler.minFilter = VK_FILTER_LINEAR;
	sampler.anisotropyEnable = VK_TRUE;
	sampler.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	vkCreateSampler(device, &sampler, nullptr, &SamplerLinear);

	VkDescriptorImageInfo textureInfo = {};
	textureInfo.sampler = SamplerLinear;  // Your VkSampler object
	textureInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet samplerWrite = {};
	samplerWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	samplerWrite.dstBinding = 0;  // Binding for sampler (in shader)
	samplerWrite.dstSet = SamplerDescriptor;  // The allocated descriptor set for the sampler
	samplerWrite.descriptorCount = 1;
	samplerWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	samplerWrite.pImageInfo = &textureInfo;  // Sampler info (same as texture)

	vkUpdateDescriptorSets(device, 1, &samplerWrite, 0, nullptr);
}

void GPUResources::RecreateBindlessTextures()
{
	// Ideally wed have update bit enabled ondescriptors
	//vkQueueWaitIdle(VkRenderer::Get().GPUQueue);

	if (!TexturesDescriptor)
	{
		CreateBindlessLayout();
	}

	BindlessTextureMapping.clear();

	std::vector<VkDescriptorImageInfo> imageInfos(Images.size());
	auto allTextures = GetAllTextures();
	for (size_t i = 0; i < Images.size(); i++) 
	{
		imageInfos[i].imageView = allTextures[i]->GetImageView();
		imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		BindlessTextureMapping[allTextures[i]->GetID()] = i;
	}

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = TexturesDescriptor;
	write.dstBinding = 0; // Binding 0
	write.dstArrayElement = 0;
	write.descriptorCount = static_cast<uint32_t>(imageInfos.size());
	write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	write.pImageInfo = imageInfos.data();

	vkUpdateDescriptorSets(VkRenderer::Get().GetDevice(), 1, &write, 0, nullptr);
}

void GPUResources::RecreateBindlessCameras()
{
	if (!CamerasDescriptor)
	{
		CreateBindlessLayout();
	}

	int i = 0;
	for (auto& c : Cameras)
	{
		CamerasData[i] = c;
		i++;
	}

	void* mappedData;
	auto allocator = VulkanAllocator::Get().GetAllocator();
	vmaMapMemory(allocator, (VkRenderer::Get().GetCurrentFrame().CamerasStagingBuffer->GetAllocation()), &mappedData);
	memcpy(mappedData, Cameras.data(), sizeof(CameraView) * Cameras.size());

	VkRenderer::Get().ImmediateSubmit([&](VkCommandBuffer cmd) {
		VkBufferCopy copy{ 0 };
		copy.dstOffset = 0;
		copy.srcOffset = 0;
		copy.size = sizeof(CameraView) * MAX_CAMERAS;

		vkCmdCopyBuffer(cmd, VkRenderer::Get().GetCurrentFrame().CamerasStagingBuffer->GetBuffer(), CamerasBuffer->GetBuffer(), 1, &copy);
	});

	vmaUnmapMemory(allocator, VkRenderer::Get().GetCurrentFrame().CamerasStagingBuffer->GetAllocation());

	// Update descriptor set for camera
	VkDescriptorBufferInfo transformBufferInfo{};
	transformBufferInfo.buffer = CamerasBuffer->GetBuffer();
	transformBufferInfo.offset = 0;
	transformBufferInfo.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet bufferWriteModel = {};
	bufferWriteModel.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	bufferWriteModel.pNext = nullptr;
	bufferWriteModel.dstBinding = 0;
	bufferWriteModel.dstSet = CamerasDescriptor;
	bufferWriteModel.descriptorCount = 1;
	bufferWriteModel.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bufferWriteModel.pBufferInfo = &transformBufferInfo;
	vkUpdateDescriptorSets(VkRenderer::Get().GetDevice(), 1, &bufferWriteModel, 0, nullptr);
}

void GPUResources::UpdateBuffers()
{
	// Tranforms
	{
		void* mappedData;
		vmaMapMemory(VulkanAllocator::Get().GetAllocator(), (VkRenderer::Get().GetCurrentFrame().ModelStagingBuffer->GetAllocation()), &mappedData);
		memcpy(mappedData, &ModelTransforms, sizeof(ModelData));

		VkRenderer::Get().ImmediateSubmit([&](VkCommandBuffer cmd) {
			VkBufferCopy copy{ 0 };
			copy.dstOffset = 0;
			copy.srcOffset = 0;
			copy.size = sizeof(ModelData);

			vkCmdCopyBuffer(cmd, VkRenderer::Get().GetCurrentFrame().ModelStagingBuffer->GetBuffer(), ModelBuffer->GetBuffer(), 1, &copy);
		});

		vmaUnmapMemory(VulkanAllocator::Get().GetAllocator(), VkRenderer::Get().GetCurrentFrame().ModelStagingBuffer->GetAllocation());

		// Update descriptor set for camera
		VkDescriptorBufferInfo transformBufferInfo{};
		transformBufferInfo.buffer = ModelBuffer->GetBuffer();
		transformBufferInfo.offset = 0;
		transformBufferInfo.range = VK_WHOLE_SIZE;

		VkWriteDescriptorSet bufferWriteModel = {};
		bufferWriteModel.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		bufferWriteModel.pNext = nullptr;
		bufferWriteModel.dstBinding = 0;
		bufferWriteModel.dstSet = ModelDescriptor;
		bufferWriteModel.descriptorCount = 1;
		bufferWriteModel.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		bufferWriteModel.pBufferInfo = &transformBufferInfo;
		vkUpdateDescriptorSets(VkRenderer::Get().GetDevice(), 1, &bufferWriteModel, 0, nullptr);
	}

	// Update material buffer
	{
		void* mappedData;
		vmaMapMemory(VulkanAllocator::Get().GetAllocator(), (VkRenderer::Get().GetCurrentFrame().MaterialStagingBuffer->GetAllocation()), &mappedData);
		memcpy(mappedData, &MaterialDataContainer, sizeof(MaterialData));

		VkRenderer::Get().ImmediateSubmit([&](VkCommandBuffer cmd) {
			VkBufferCopy copy{ 0 };
			copy.dstOffset = 0;
			copy.srcOffset = 0;
			copy.size = sizeof(MaterialData);

			vkCmdCopyBuffer(cmd, VkRenderer::Get().GetCurrentFrame().MaterialStagingBuffer->GetBuffer(), MaterialBuffer->GetBuffer(), 1, &copy);
		});

		vmaUnmapMemory(VulkanAllocator::Get().GetAllocator(), VkRenderer::Get().GetCurrentFrame().MaterialStagingBuffer->GetAllocation());

		// Update descriptor set for camera
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = MaterialBuffer->GetBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = VK_WHOLE_SIZE;

		VkWriteDescriptorSet bufferWrite = {};
		bufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		bufferWrite.pNext = nullptr;
		bufferWrite.dstBinding = 0;
		bufferWrite.dstSet = MaterialDescriptor;
		bufferWrite.descriptorCount = 1;
		bufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		bufferWrite.pBufferInfo = &bufferInfo;
		bufferWrite.pImageInfo = VK_NULL_HANDLE;
		vkUpdateDescriptorSets(VkRenderer::Get().GetDevice(), 1, &bufferWrite, 0, nullptr);
	}

	// Lights
	{
		void* mappedData;
		vmaMapMemory(VulkanAllocator::Get().GetAllocator(), (VkRenderer::Get().GetCurrentFrame().LightStagingBuffer->GetAllocation()), &mappedData);
		memcpy(mappedData, &LightDataContainerArray, sizeof(LightDataContainer));

		VkRenderer::Get().ImmediateSubmit([&](VkCommandBuffer cmd) {
			VkBufferCopy copy{ 0 };
			copy.dstOffset = 0;
			copy.srcOffset = 0;
			copy.size = sizeof(LightDataContainer);

			vkCmdCopyBuffer(cmd, VkRenderer::Get().GetCurrentFrame().LightStagingBuffer->GetBuffer(), LightBuffer->GetBuffer(), 1, &copy);
		});

		vmaUnmapMemory(VulkanAllocator::Get().GetAllocator(), VkRenderer::Get().GetCurrentFrame().LightStagingBuffer->GetAllocation());

		// Update descriptor set for camera
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = LightBuffer->GetBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = VK_WHOLE_SIZE;

		VkWriteDescriptorSet bufferWrite = {};
		bufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		bufferWrite.pNext = nullptr;
		bufferWrite.dstBinding = 0;
		bufferWrite.dstSet = LightsDescriptor;
		bufferWrite.descriptorCount = 1;
		bufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		bufferWrite.pBufferInfo = &bufferInfo;
		bufferWrite.pImageInfo = VK_NULL_HANDLE;
		vkUpdateDescriptorSets(VkRenderer::Get().GetDevice(), 1, &bufferWrite, 0, nullptr);
	}
}

std::vector<VkDescriptorSetLayout> GPUResources::GetBindlessLayout()
{
	std::vector<VkDescriptorSetLayout> layouts = {
		ModelDescriptorLayout,
		TriangleBufferDescriptorLayout,
		SamplerDescriptorLayout,
		MaterialDescriptorLayout,
		TexturesDescriptorLayout,
		LightsDescriptorLayout,
		CamerasDescriptorLayout,
		SSAOKernelDescriptorLayout
	};
	return layouts;
}

std::vector<VkDescriptorSet> GPUResources::GetBindlessDescriptorSets()
{
	std::vector<VkDescriptorSet> descriptors = {
		ModelDescriptor,
		TriangleBufferDescriptor,
		SamplerDescriptor,
		MaterialDescriptor,
		TexturesDescriptor,
		LightsDescriptor,
		CamerasDescriptor
	};
	return descriptors;
}

uint32_t GPUResources::GetBindlessTextureID(const UUID& id)
{
	if (BindlessTextureMapping.find(id) == BindlessTextureMapping.end())
	{
		return 0;
	}

	return BindlessTextureMapping[id];
}

uint32_t GPUResources::GetBindlessCameraID(const UUID& id)
{
	if (CameraMapping.find(id) == CameraMapping.end())
	{
		return 0;
	}

	return CameraMapping[id];
}

uint32_t GPUResources::GetBindlessTransformID(const UUID& id)
{
	if (ModelMatrixMapping.find(id) == ModelMatrixMapping.end())
	{
		return 0;
	}

	return ModelMatrixMapping[id];
}

uint32_t GPUResources::GetBindlessMaterialID(const UUID& id)
{
	if (MeshMaterialMapping.find(id) == MeshMaterialMapping.end())
	{
		return 0;
	}
	return MeshMaterialMapping[id];
}

void GPUResources::QueueDeletion(CleanUpStack cleanUpStack)
{
	CleanUpStack& deletionQueue = GetFrameCleanUpStack(VkRenderer::Get().FrameNumber);
	// Push content of cleanUpStack into deletionQueue
	while (!cleanUpStack.empty())
	{
		deletionQueue.push(cleanUpStack.top());
		cleanUpStack.pop();
	}
}

void GPUResources::CleanUp(uint32_t frame)
{
	auto& deletionQueue = GetFrameCleanUpStack(frame);
	if (!deletionQueue.empty())
	{
		vkQueueWaitIdle(VkRenderer::Get().GPUQueue);
	}

	while (!deletionQueue.empty())
	{
		deletionQueue.top()();
		deletionQueue.pop();
	}
}

CleanUpStack& GPUResources::GetFrameCleanUpStack(uint32_t frame)
{
	return DeletionQueue[frame % FRAME_OVERLAP];
}