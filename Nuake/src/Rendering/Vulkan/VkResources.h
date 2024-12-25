#pragma once
#include "src/Core/Core.h"
#include "src/Resource/UUID.h"

#include "VulkanAllocatedBuffer.h"

namespace Nuake
{
	class GPUResources
	{
	private:
		std::map<UUID, Ref<AllocatedBuffer>> Buffers;

	public:
		static GPUResources& Get()
		{
			static GPUResources instance;
			return instance;
		};

		GPUResources() = default;
		~GPUResources() = default;

		bool AddBuffer(const UUID& id, const Ref<AllocatedBuffer>& buffer)
		{
			if (Buffers.find(id) == Buffers.end())
			{
				Buffers[id] = buffer;
				return true;
			}

			return false;
		}

		Ref<AllocatedBuffer> GetBuffer(const UUID& id)
		{
			if (Buffers.find(id) != Buffers.end())
			{
				return Buffers[id];
			}

			return nullptr;
		}
	};
}