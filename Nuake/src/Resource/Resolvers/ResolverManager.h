#pragma once
#include "IResourceResolver.h"

#include "src/Core/Core.h"
#include "src/Core/Logger.h"
#include "src/FileSystem/File.h"

#include <map>
#include <string>

namespace Nuake
{
	class ResourceResolverManager
	{
	private:
		std::map<std::string, Ref<IResourceResolver>> Resolvers;

	public:
		static ResourceResolverManager& Get()
		{
			static ResourceResolverManager instance;
			return instance;
		}

		void RegisterResolver(Ref<IResourceResolver> resolver)
		{
			Resolvers[resolver->GetExtension()] = resolver;
		}

		bool IsFileTypeResolvable(const std::string& extension)
		{
			return Resolvers.find(extension) != Resolvers.end();
		}

		Ref<IResourceResolver> GetResolverForFile(const Ref<File>& file)
		{
			const std::string extension = file->GetExtension();
			if (IsFileTypeResolvable(extension))
			{
				return Resolvers[extension];
			}

			Logger::Log("Failed to find resolver for file type: " + extension, "resolver", CRITICAL);
			return nullptr;
		}

		// This will only read the UUID of a resource file
		UUID ResolveUUID(const Ref<File> file)
		{
			Ref<IResourceResolver> resolver = GetResolverForFile(file);
			if (resolver)
			{
				return resolver->ResolveUUID(file);
			}

			return UUID(0);
		}

		Ref<Resource> Resolve(const Ref<File> file)
		{
			Ref<IResourceResolver> resolver = GetResolverForFile(file);
			if (resolver)
			{
				return resolver->Resolve(file);
			}
			
			Logger::Log("Failed to resolve file: " + file->GetRelativePath(), "resolver", CRITICAL);
			return nullptr;
		}
	};
}
