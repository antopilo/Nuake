#pragma once
#include "Nuake/Core/Core.h"
#include "Nuake/Resource/UUID.h"

#include "Nuake/Resource/ResourceManager.h"

namespace Nuake
{
	class Resource;

	class RID
	{
	private:
		struct RemapRequest
		{
			UUID Old;
			UUID New;
		};

		static std::vector<RID*> Handles;
		static std::queue<RemapRequest> RemapQueue;
		static std::mutex RemapMutex;

	public:
		UUID ID;
		Ref<Resource> Data;

		RID() : ID(0), Data(nullptr) 
		{
			Handles.push_back(this);
		}
		RID(UUID id) : ID(id) 
		{
			Handles.push_back(this);
		}
		
		~RID()
		{
			std::remove_if(Handles.begin(), Handles.end(), [this](const RID* rid) { return rid == this; });
		}

		static void ExecuteRemaps()
		{
			std::lock_guard<std::mutex> lock(RemapMutex);
			while (!RemapQueue.empty())
			{
				auto& request = RemapQueue.front();
				uint32_t remappedCount = 0;
				for (auto& handle : Handles)
				{
					if (handle->ID == request.Old)
					{
						handle->ID = request.New;

						if (handle->Data)
						{
							handle->Data = nullptr;
						}

						remappedCount++;
					}
				}
				RemapQueue.pop();

				Logger::Log("Remapped " + std::to_string(remappedCount), "RID", VERBOSE);
			}
		}

		static void QueueRemap(const UUID& old, const UUID& newUUID)
		{
			std::lock_guard<std::mutex> lock(RemapMutex);
			if (old == newUUID)
			{
				return;
			}

			RemapQueue.push({ old, newUUID });
		}

	public:
		template<typename T>
		Ref<T> Get()
		{
			if (!Data)
			{
				Load<T>();
			}

			return std::static_pointer_cast<T>(Data);
		}

		template<typename T>
		void Load()
		{
			if (!Data)
			{
				Data = ResourceManager::GetResource<T>(ID);
			}
		}
	};
}