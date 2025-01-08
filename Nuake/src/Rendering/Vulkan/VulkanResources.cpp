#include "VkResources.h"

using namespace Nuake;

GPUResources::GPUResources()
{
	Init();
}

void GPUResources::Init()
{
	CreateBindlessLayout();

}

Ref<AllocatedBuffer> GPUResources::CreateBuffer(size_t size, BufferUsage flags, MemoryUsage usage, const std::string& name)
{
	Ref<AllocatedBuffer> buffer = CreateRef<AllocatedBuffer>(name, size, flags, usage);
	Buffers[buffer->GetID()] = buffer;
	return buffer;
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

Ref<VulkanImage> GPUResources::GetTexture(const UUID& id)
{
	if (Images.find(id) != Images.end())
	{
		return Images[id];
	}

	Logger::Log("Mesh with ID does not exist", "vulkan", CRITICAL);
	return TextureManager::Get()->GetTexture2("missing_texture");
}

void GPUResources::CreateBindlessLayout()
{
	auto& vk = VkRenderer::Get();
	auto device = vk.GetDevice();

	// Camera
	{
		DescriptorLayoutBuilder builder;
		builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		CameraDescriptorLayout = builder.Build(device, VK_SHADER_STAGE_VERTEX_BIT);
	}

	{
		// Triangle vertex buffer layout
		DescriptorLayoutBuilder builder;
		builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		TriangleBufferDescriptorLayout = builder.Build(device, VK_SHADER_STAGE_VERTEX_BIT);
	}

	{
		// Matrices
		DescriptorLayoutBuilder builder;
		builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		ModelBufferDescriptorLayout = builder.Build(device, VK_SHADER_STAGE_VERTEX_BIT);
	}

	// Textures
	{
		DescriptorLayoutBuilder builder;
		builder.AddBinding(0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
		ImageDescriptorLayout = builder.Build(device, VK_SHADER_STAGE_FRAGMENT_BIT);
	}

	{
		DescriptorLayoutBuilder builder;
		builder.AddBinding(0, VK_DESCRIPTOR_TYPE_SAMPLER);
		SamplerDescriptorLayout = builder.Build(device, VK_SHADER_STAGE_FRAGMENT_BIT);
	}

	// Material
	{
		DescriptorLayoutBuilder builder;
		builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		MaterialDescriptorLayout = builder.Build(device, VK_SHADER_STAGE_FRAGMENT_BIT);
	}
}

std::vector<VkDescriptorSetLayout> GPUResources::GetBindlessLayout()
{
	std::vector<VkDescriptorSetLayout> layouts = {
		CameraDescriptorLayout,
		ModelBufferDescriptorLayout,
		TriangleBufferDescriptorLayout,
		ImageDescriptorLayout,
		SamplerDescriptorLayout,
		MaterialDescriptorLayout
	};
	return layouts;
}