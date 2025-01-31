#pragma once

#include "Nuake/Resource/UUID.h"
#include "Nuake/Resource/Serializable.h"
#include "Nuake/FileSystem/FileSystem.h"

#include <map>
#include <string>

namespace Nuake
{
	// This class holds the mapping between resource UUIDs and their paths.
	// Ideally, this is the first steps for a VFS for shipped games
	class ResourceManifest
	{
	private:
		std::map<std::string, UUID> PathToMap;
		std::map<UUID, std::string> MapToPath;

	public:
		void RegisterResource(UUID uuid, const std::string& path)
		{
			// New resource is overwritting so we need to clear UUID -> Path map
			if (PathToMap.find(path) != PathToMap.end())
			{
				MapToPath.erase(PathToMap[path]);
			}

			PathToMap[path] = uuid;
			MapToPath[uuid] = path;
		}

		std::string GetResourcePath(UUID uuid)
		{
			if (MapToPath.find(uuid) != MapToPath.end())
			{
				return MapToPath[uuid];
			}

			return "";
		}

		UUID GetResourceUUID(const std::string& path)
		{
			if (PathToMap.find(path) != PathToMap.end())
			{
				return PathToMap[path];
			}

			return UUID(0);
		}

		void Serialize(const std::string& path)
		{
			json j;
			for (auto& [path, uuid] : PathToMap)
			{
				j[path] = static_cast<uint64_t>(uuid);
			}

			FileSystem::BeginWriteFile(path);
			FileSystem::WriteLine(j.dump(4));
			FileSystem::EndWriteFile();
		}

		void Deserialize(const std::string& path)
		{
			auto content = FileSystem::ReadFile(path);
			json j = json::parse(content);
			for (auto& [path, uuid] : j.items())
			{
				uint64_t id = uuid;
				PathToMap[path] = UUID(id);
				MapToPath[UUID(id)] = path;
			}
		}
	};
}