#pragma once
#include "src/Core/Core.h"

#include "src/FileSystem/FileSystem.h"
#include "src/FileSystem/File.h"

#include "src/Resource/Resource.h"
#include "src/Resource/ResourceManifest.h"
#include "src/Resource/Serializer/BinarySerializer.h"
#include "src/Resource/UUID.h"

#include "src/Resource/Resolvers/ResolverManager.h"

#include <map>


namespace Nuake 
{
	class ResourceManager 
	{
	private:
		static std::map<UUID, Ref<Resource>> m_Resources;

	public:
		static ResourceManifest Manifest;
		static Ref<Resource> GetResource(const UUID& uuid);

		static void RegisterResource(Ref<Resource> resource)
		{
			m_Resources[resource->ID] = resource;
		}

		static bool IsResourceLoaded(const UUID& uuid)
		{
			return m_Resources.find(uuid) != m_Resources.end();
		}

		static Ref<Resource> RegisterUnregisteredRessource(Ref<File> file)
		{
			Ref<Resource> resolvedResource = ResourceResolverManager::Get().Resolve(file);
			RegisterResource(resolvedResource);
			Manifest.RegisterResource(resolvedResource->ID, file->GetRelativePath());
			return resolvedResource;
		}

		template<typename R>
		static Ref<R> GetResource(const UUID& uuid)
		{
			Ref<R> resource;
			if (m_Resources.find(uuid) == m_Resources.end())
			{
				// We need to load from disk
				auto resourcePath = Manifest.GetResourcePath(uuid);
				if (resourcePath.empty())
				{
					//assert(false && "Resource not found in manifest");
					return nullptr;
				}

				if (!FileSystem::FileExists(resourcePath))
				{
					return nullptr;
				}

				Ref<File> file = FileSystem::GetFile(resourcePath);
				if (file)
				{
					resource = std::static_pointer_cast<R>(RegisterUnregisteredRessource(file));
				}
			}
			else
			{
				resource = std::static_pointer_cast<R>(m_Resources[uuid]);
			}

			return resource;
		}

		template<typename R>
		static Ref<R> GetResourceFromFile(Ref<File> file)
		{
			if (UUID id = Manifest.GetResourceUUID(file->GetRelativePath()); id != 0)
			{
				if (IsResourceLoaded(id))
				{
					return std::static_pointer_cast<R>(m_Resources[id]);
				}

				// Resource is in manifest, but not loaded? Corruption?
			}

			auto resource = RegisterUnregisteredRessource(file);
			return std::static_pointer_cast<R>(resource);
		}
	};
}