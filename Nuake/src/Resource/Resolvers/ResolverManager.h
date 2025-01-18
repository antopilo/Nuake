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

		Ref<Resource> Resolve(const Ref<File> file)
		{
			std::string extension = file->GetExtension();
			if (Resolvers.find(extension) != Resolvers.end())
			{
				return Resolvers[extension]->Resolve(file);
			}

			Logger::Log("Failed to find resolver for file type: " + extension, "resolver", CRITICAL);
			return nullptr;
		}
	};
}
