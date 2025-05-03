#include "BindlessDescriptor.h"

#include "Nuake/Core/Logger.h"

#include "VulkanAllocator.h"
#include "VulkanRenderer.h"
#include "VulkanInit.h"
#include "DescriptorLayoutBuilder.h"

#include "GPUData/GPUData.h"

using namespace Nuake;

std::string GetResourceTypeName(ResourceType type)
{
	switch (type)
	{
	case ResourceType::View: return "View";
	case ResourceType::Material: return "Material";
	case ResourceType::Texture: return "Texture";
	case ResourceType::Light: return "Light";
	case ResourceType::Sampler: return "Sampler";
	default: return "Unknown";
	}
}

int32_t Descriptor::LoadResource(const UUID& id)
{
	// Check if we have already loaded this resource
	if (SlotMapping.find(id) != SlotMapping.end())
	{
		return SlotMapping[id];
	}

	// Find an empty slot
	for (int i = 0; i < Slots.size(); i++)
	{
		if (!Slots[i].Active)
		{
			DescriptorSlot& slot = Slots[i];
			slot.Active = true;
			slot.Slot = i;
			SlotMapping[id] = i;
			return i;
		}
	}

	return -1;
}

int32_t Descriptor::GetResourceSlot(const UUID& id) const
{
	auto it = SlotMapping.find(id);
	if (it != SlotMapping.end())
	{
		return it->second;
	}
	return 0; // Resource not found
}

Descriptor::Descriptor(Ref<AllocatedBuffer> buffer, VkDescriptorSetLayout layout, uint8_t* ptr, size_t offset, size_t size, BindlessInfo& info)
	: DataPtr(ptr), Offset(offset), Size(size), Info(info)
{
	auto& vk = VkRenderer::Get();
	auto& allocator = VkRenderer::Get().GetDescriptorAllocator();
	DescriptorSet = allocator.Allocate(vk.GetDevice(), layout);

	// Offset since we are splitting the buffer by N frames in flight
	// Each partition is a frame in flight
	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = buffer->GetBuffer();
	bufferInfo.offset = offset;
	bufferInfo.range = size;

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = DescriptorSet;
	write.dstBinding = 0;
	write.dstArrayElement = 0;
	write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	write.descriptorCount = 1;
	write.pBufferInfo = &bufferInfo;

	vkUpdateDescriptorSets(vk.GetDevice(), 1, &write, 0, nullptr);
}

BindlessDescriptor::BindlessDescriptor(ResourceType type, BindlessInfo& info) :
	Info(info), 
	Type(type)
{
	// Create a buffer that holds for N frame in flights of data
	const std::string& resourceName = GetResourceTypeName(type);
	// Build descriptor layout
	DescriptorLayoutBuilder builder;
	switch (type)
	{
	case ResourceType::View:
	case ResourceType::Material:
	case ResourceType::Light:
		builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		break;
	case ResourceType::Texture:
		builder.AddBinding(0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
		break;
	}

	DescriptorLayout = builder.Build(VkRenderer::Get().GetDevice(), VK_SHADER_STAGE_ALL_GRAPHICS);
	VulkanUtil::SetDebugName(DescriptorLayout, resourceName + "DescriptorLayout");

	// Create buffer and map, CPU -> GPU since we will writing to it directly
	// Size of buffer is: size_of(ResourceType) * FRAME_OVERLAP since 1 buffer will hold N Frames in flight
	// TODO(antopilo): move mapped pointer inside buffer directly
	const BufferUsage usage = BufferUsage::STORAGE_BUFFER | BufferUsage::TRANSFER_DST;
	const MemoryUsage memoryUsage = MemoryUsage::CPU_TO_GPU;
	const size_t size = info.ResourceElementSize[type] * info.ResourceCount[type];
	const size_t totalSize = size * FRAME_OVERLAP;
	Buffer = CreateRef<AllocatedBuffer>(resourceName + "GPUBuffer", totalSize, usage, memoryUsage);

	// Map to a host-visible pointer
	void* mappedData;
	vmaMapMemory(VulkanAllocator::Get().GetAllocator(), (Buffer->GetAllocation()), &mappedData);
	for (int i = 0; i < FRAME_OVERLAP; i++)
	{
		const size_t offset = i * size;
		uint8_t* partitionStart = static_cast<uint8_t*>(mappedData) + offset;
		Descriptors.emplace_back(Buffer, DescriptorLayout, partitionStart, offset, size, info);
	}
}

void BindlessDescriptor::WriteToBuffer(int32_t frameIndex, void* data, size_t size)
{
	int currentFrame = frameIndex % FRAME_OVERLAP;
	auto& desc = Descriptors[currentFrame];
	
	const size_t offsetSize = Info.ResourceCount[Type] * Info.ResourceElementSize[Type];
	size_t offset = currentFrame * offsetSize;
	
	memcpy(desc.DataPtr, data, size);
}

void BindlessDescriptor::Swap(int32_t frameIndex)
{
	int currentFrame = frameIndex % FRAME_OVERLAP;
	int nextFrame = (frameIndex + 1) % FRAME_OVERLAP;
	auto& desc = Descriptors[currentFrame];
	auto& nextDesc = Descriptors[nextFrame];

	// memcpy from currentFrame to next frame
	memcpy(desc.DataPtr, nextDesc.DataPtr, desc.Size);
}

ResourceDescriptors::ResourceDescriptors(const ResourceDescriptorsLimits& limits)
{
	AddResourceDescriptors<ResourceType::View, CameraView>(limits.MaxView);
	AddResourceDescriptors<ResourceType::Material, MaterialBufferStruct>(limits.MaxMaterial);
	AddResourceDescriptors<ResourceType::Texture, VkDescriptorImageInfo>(limits.MaxTexture);
	AddResourceDescriptors<ResourceType::Light, LightData>(limits.MaxLight);
	AddResourceDescriptors<ResourceType::Sampler, VkDescriptorImageInfo>(limits.MaxSampler);
}

void ResourceDescriptors::Swap(int32_t frameIndex)
{
	for (auto& [type, desc] : Descriptors)
	{
		desc.Swap(frameIndex);
	}
}