#pragma once

#include "Nuake/Core/Core.h"
#include "VulkanAllocatedBuffer.h"
#include "VulkanRenderer.h"

#include <volk/volk.h>

#include <map>

namespace Nuake
{
	enum class ResourceType
	{
		View,
		Material,
		Texture,
		Light,
		Sampler
	};

	struct BindlessInfo
	{
		std::map<ResourceType, size_t> ResourceElementSize;
		std::map<ResourceType, size_t> ResourceCount;
		std::map<ResourceType, AllocatedBuffer> ResourceBuffers;
	};

	class DescriptorSlot
	{
	public:
		bool Active;
		int32_t Slot;
		UUID RID;
	};

	// A partition of the buffer in BindlessDescriptor
	class Descriptor
	{
	private:
		uint8_t* DataPtr;
		size_t Size;
		size_t Offset;

	private:
		BindlessInfo& Info;

		VkDescriptorSet DescriptorSet;
		std::map<UUID, int> SlotMapping;
		std::vector<DescriptorSlot> Slots;

	public:
		Descriptor(Ref<AllocatedBuffer> buffer, VkDescriptorSetLayout layout, uint8_t* ptr, size_t offset, size_t size, BindlessInfo& info);
		~Descriptor() = default;

		int32_t LoadResource(const UUID& id);
		int32_t GetResourceSlot(const UUID& id) const;
	};

	// Contains buffer for N frames of a resource type
	class BindlessDescriptor
	{
	private:
		Ref<AllocatedBuffer> Buffer;
		std::vector<Descriptor> Descriptors;
		VkDescriptorSetLayout DescriptorLayout;

	public:
		BindlessDescriptor(ResourceType type, BindlessInfo& info);
		BindlessDescriptor() = default;
		~BindlessDescriptor() = default;

		void QueueCopy(const size_t frameIndex, const size_t offset, const size_t size, const void* data);
	};

	struct ResourceDescriptorsLimits
	{
		size_t MaxView;
		size_t MaxMaterial;
		size_t MaxTexture;
		size_t MaxLight;
		size_t MaxSampler;
	};

	// Contains all buffers per resource
	class ResourceDescriptors
	{
	private:
		std::map<ResourceType, BindlessDescriptor> Descriptors;
		BindlessInfo Info;
	public:
		ResourceDescriptors(const ResourceDescriptorsLimits& limits);
		~ResourceDescriptors() = default;

		template<ResourceType T, typename S>
		void AddResourceDescriptors(const size_t size)
		{
			Info.ResourceElementSize[T] = sizeof(S);
			Info.ResourceCount[T] = size;
			//Descriptors[T] = BindlessDescriptor(T, Info);
		}
	};
}